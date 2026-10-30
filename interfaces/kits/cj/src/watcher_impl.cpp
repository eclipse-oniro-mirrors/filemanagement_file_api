/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "watcher_impl.h"
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include "securec.h"

using namespace OHOS::CJSystemapi::FileFs;

namespace OHOS::CJSystemapi {
using namespace std;

mutex FileWatcherManager::watchMutex_;

FileWatcherManager::FileWatcherManager() {}

FileWatcherManager::~FileWatcherManager()
{
    // Best-effort reclaim of the poll thread: destroying a still joinable
    // std::thread member would call std::terminate during static teardown.
    if (pollState_) {
        pollState_->stopRequested.store(true);
    }
    if (eventFd_ >= 0) {
        uint64_t wakeVal = 1;
        (void)write(eventFd_, &wakeVal, sizeof(wakeVal));
    }
    if (pollThread_.joinable()) {
        pollThread_.join();
    }
}

int32_t FileWatcherManager::GetNotifyId()
{
    return notifyFd_;
}

bool FileWatcherManager::InitNotify()
{
    notifyFd_ = inotify_init();
    if (notifyFd_ < 0) {
        LOGE("Failed to init notify errCode:%{public}d", errno);
        return false;
    }
    fdsan_exchange_owner_tag(notifyFd_, 0, CJ_FILE_FDSAN_TAG);
    eventFd_ = eventfd(0, EFD_CLOEXEC);
    if (eventFd_ < 0) {
        LOGE("Failed to init eventfd errCode:%{public}d", errno);
        return false;
    }
    fdsan_exchange_owner_tag(eventFd_, 0, CJ_FILE_FDSAN_TAG);
    return true;
}

bool FileWatcherManager::AddWatcherInfo(const string &fileName, shared_ptr<WatcherInfoArg> arg)
{
    // Guard the set: the poll thread iterates watcherInfoSet_ inside NotifyEvent
    // under watchMutex_, so an unguarded insert or traversal here races with it.
    lock_guard<mutex> lock(watchMutex_);
    for (auto const &iter : watcherInfoSet_) {
        if (iter->fileName == arg->fileName && iter->events == arg->events) {
            if (iter->callbackId == arg->callbackId) {
                LOGE("Faile to add watcher, fileName:%{private}s the callback is same", fileName.c_str());
                return false;
            }
        }
    }
    watcherInfoSet_.insert(arg);
    return true;
}

bool CheckIncludeEvent(const uint32_t &mask, const uint32_t &event)
{
    if ((mask & event) > 0) {
        return true;
    }
    return false;
}

uint32_t FileWatcherManager::RemoveWatcherInfo(shared_ptr<WatcherInfoArg> arg)
{
    watcherInfoSet_.erase(arg);
    uint32_t otherEvents = 0;
    for (const auto &iter : watcherInfoSet_) {
        if (iter->fileName == arg->fileName && iter->wd > 0) {
            otherEvents |= iter->events;
        }
    }
    return otherEvents;
}

int32_t FileWatcherManager::StartNotify(std::shared_ptr<WatcherInfoArg> arg)
{
    lock_guard<mutex> lock(watchMutex_);
    if (notifyFd_ < 0) {
        LOGE("Failed to start notify notifyFd_:%{public}d", notifyFd_);
        return GetErrorCode(EIO);
    }
    // Always (re)add the watch instead of trusting the recorded state: the kernel
    // returns the same wd for an existing watch and transparently creates a fresh
    // one if the old watch was auto-removed (e.g. the file was deleted and
    // recreated). A stale wdFileNameMap_ entry must never turn start() into a
    // no-op, otherwise the watcher silently keeps a dead watch descriptor.
    uint32_t watchEvents = arg->events;
    for (const auto &iter : watcherInfoSet_) {
        if (iter->fileName == arg->fileName && iter->wd > 0) {
            watchEvents |= iter->events;
        }
    }
    int newWd = inotify_add_watch(notifyFd_, arg->fileName.c_str(), watchEvents);
    if (newWd < 0) {
        LOGE("Failed to start notify errCode:%{public}d", errno);
        return GetErrorCode(errno);
    }
    arg->wd = newWd;
    // Record whether the watch target is a directory. The kernel attaches a
    // child name to an event in more cases than "the watch is on a directory":
    // on the 5.10 kernel, when a mount/superblock mark (e.g. an external
    // fanotify listener) makes fsnotify set parent_needed without
    // parent_interested, the name is carried WITHOUT FS_EVENT_ON_CHILD, and
    // fsnotify_handle_event() then delivers it - name intact - to the watched
    // file's own mark. Appending that name to a file watch's own path would
    // fabricate "path/basename(path)", so the append must be gated on isDir.
    struct stat st = {};
    arg->isDir = (stat(arg->fileName.c_str(), &st) == 0) && S_ISDIR(st.st_mode);
    wdFileNameMap_[arg->fileName].first = newWd;
    wdFileNameMap_[arg->fileName].second = watchEvents;
    RemoveStaleWdEntries(arg->fileName, newWd);
    return SUCCESS_CODE;
}

int FileWatcherManager::NotifyToWatchNewEvents(const string &fileName, const uint32_t &watchEvents)
{
    int newWd = inotify_add_watch(notifyFd_, fileName.c_str(), watchEvents);
    if (newWd < 0) {
        LOGE("Failed to start new notify errCode:%{public}d", errno);
        return GetErrorCode(errno);
    }
    // The previous watch may have been auto-removed by the kernel (the watched
    // file was deleted and recreated): accept the new wd and refresh every live
    // watcher on this path instead of failing the stop.
    wdFileNameMap_[fileName].first = newWd;
    for (const auto &iter : watcherInfoSet_) {
        if (iter->fileName == fileName && iter->wd > 0) {
            iter->wd = newWd;
        }
    }
    wdFileNameMap_[fileName].second = watchEvents;
    RemoveStaleWdEntries(fileName, newWd);
    return SUCCESS_CODE;
}

void FileWatcherManager::RemoveStaleWdEntries(const std::string &fileName, const int wd)
{
    for (auto it = wdFileNameMap_.begin(); it != wdFileNameMap_.end();) {
        if (it->first != fileName && it->second.first == wd) {
            it = wdFileNameMap_.erase(it);
        } else {
            ++it;
        }
    }
}

void FileWatcherManager::ReadNotifyEvent(int32_t notifyFd)
{
    // Read and parse under the mutex: CloseNotifyFd closes the fds under this
    // same mutex (after setting stopRequested), so once the lock is acquired
    // here the fd number below cannot be in the closed-and-reused state.
    // Reading a reused fd number would feed arbitrary non-inotify bytes into
    // the parser and fabricate events the kernel never generated.
    // Listener dispatch is deliberately outside the locked section: the
    // Cangjie listener runs synchronously on this poll thread, and a listener
    // reacting to an event with stop()/start()/createWatcher() would deadlock
    // on the mutex this thread would otherwise still hold. Only owned copies
    // (shared_ptr + string) leave the locked section; a watcher stopped after
    // the snapshot may still receive one in-flight event, matching the ArkTS
    // watcher behavior.
    vector<NotifyTarget> targets;
    {
        lock_guard<mutex> lock(watchMutex_);
        // Re-check under the mutex: the fds may have been closed between the
        // poll wakeup and this lock acquisition.
        if (pollState_ && pollState_->stopRequested.load()) {
            return;
        }
        int len = 0;
        int index = 0;
        char buf[BUF_SIZE] = {0};
        struct inotify_event *event = nullptr;
        while (((len = read(notifyFd, &buf, sizeof(buf))) < 0) && (errno == EINTR)) {};
        uint32_t eventSize = static_cast<uint32_t>(sizeof(struct inotify_event));
        while (index < len) {
            // Defensive bounds checks: never walk past the buffer even if a read
            // returned a truncated record. Both ArkTS watcher implementations guard
            // the same way.
            uint32_t remain = static_cast<uint32_t>(len) - static_cast<uint32_t>(index);
            if (remain < eventSize) {
                LOGE("Out of bounds access, len:%{public}d, index:%{public}d", len, index);
                break;
            }
            event = reinterpret_cast<inotify_event *>(buf + index);
            if (event->len > remain - eventSize) {
                LOGE("Out of bounds access, len:%{public}d, index:%{public}d, eventNameLen:%{public}u",
                    len, index, event->len);
                break;
            }
            NotifyEvent(event, targets);
            index += sizeof(struct inotify_event) + static_cast<int>(event->len);
        }
    }
    DispatchNotifyEvents(targets);
}

void FileWatcherManager::GetNotifyEvent()
{
    lock_guard<mutex> lock(watchMutex_);
    if (pollState_ && !pollState_->stopRequested.load() && !pollState_->exited.load()) {
        return;
    }
    if (pollThread_.joinable()) {
        // The previous poll thread exited through an error path and was not
        // reclaimed by CloseNotifyFd. exited is stored as the thread's last
        // statement, so this join can only ever reap a finished thread.
        pollThread_.join();
    }
    auto state = std::make_shared<PollState>();
    pollState_ = state;
    int32_t notifyFd = notifyFd_;
    int32_t eventFd = eventFd_;
    // Note: this TU is built with -fno-exceptions, so std::thread construction
    // failure aborts, consistent with the rest of the module.
    pollThread_ = std::thread(&FileWatcherManager::PollNotifyLoop, this, state, notifyFd, eventFd);
}

void FileWatcherManager::PollNotifyLoop(std::shared_ptr<PollState> state, int32_t notifyFd, int32_t eventFd)
{
    nfds_t nfds = 2;
    struct pollfd fds[2];
    fds[0].fd = eventFd;
    fds[0].events = POLLIN;
    fds[1].fd = notifyFd;
    fds[1].events = POLLIN;
    while (!state->stopRequested.load()) {
        int ret = poll(fds, nfds, -1);
        // poll() failing with EINTR only means the wait was interrupted: retry.
        if (ret < 0 && errno == EINTR) {
            continue;
        }
        if (ret < 0) {
            LOGE("Failed to poll NotifyFd, errno=%{public}d", errno);
            break;
        }
        if (ret == 0) {
            continue;
        }
        if (state->stopRequested.load()) {
            // Woken up for shutdown: do not touch the fds, they may
            // already be closed and their numbers reused.
            break;
        }
        if ((fds[0].revents & POLLNVAL) != 0 || (fds[1].revents & POLLNVAL) != 0) {
            LOGE("Failed to poll NotifyFd, fd is invalid");
            break;
        }
        if ((fds[0].revents & POLLIN) != 0) {
            // Drain the wake-up signal; the eventfd is only used to stop the thread.
            uint64_t wakeVal = 0;
            (void)read(eventFd, &wakeVal, sizeof(wakeVal));
            continue;
        }
        if ((fds[1].revents & POLLIN) != 0) {
            ReadNotifyEvent(notifyFd);
        }
    }
    state->exited.store(true);
}

void FileWatcherManager::NotifyEvent(const struct inotify_event *event, vector<NotifyTarget> &targets)
{
    // Called with watchMutex_ held (see ReadNotifyEvent); takes no lock itself.
    // Matching watchers are appended to targets as owned copies; the actual
    // listener invocation happens in DispatchNotifyEvents, after the mutex is
    // released.
    // IN_IGNORED is deliberately not acted upon. inotify_rm_watch() frees the wd
    // for reuse immediately, while the IN_IGNORED event is only read later by the
    // poll thread; by the time it is processed the wd may already belong to a
    // fresh watch, and purging routing state on the stale event would silence
    // that new watch (none of its events would match a map entry any more).
    // Stale entries are instead purged synchronously on every successful
    // inotify_add_watch, see RemoveStaleWdEntries. IN_IGNORED carries no bit
    // inside IN_ALL_EVENTS, so the matching loop below ignores it naturally.
    string tempFileName;
    auto found = find_if(wdFileNameMap_.begin(), wdFileNameMap_.end(),
        [event](const pair<std::string, std::pair<int, uint32_t>> &iter) {
            return iter.second.first == event->wd;
    });
    if (found != wdFileNameMap_.end()) {
        tempFileName = found->first;
    }

    for (const auto &iter : watcherInfoSet_) {
        string fileName = tempFileName;
        uint32_t watchEvent = 0;
        if ((iter->fileName == fileName) && (iter->wd > 0)) {
            watchEvent = iter->events;
        }
        if (!CheckIncludeEvent(event->mask, watchEvent)) {
            continue;
        }
        // Only a directory watch reports child events: for a watch on a regular
        // file the event is about the file itself, and the kernel may still
        // attach the file's own name (e.g. via a mount/superblock mark on the
        // 5.10 kernel) - appending it would fabricate "path/basename(path)".
        if (event->len > 0 && iter->isDir) {
            fileName += "/" + string(event->name);
        }
        targets.push_back({ iter, std::move(fileName), event->mask & IN_ALL_EVENTS, event->cookie });
    }
}

void FileWatcherManager::DispatchNotifyEvents(const vector<NotifyTarget> &targets)
{
    // Runs with watchMutex_ released (see ReadNotifyEvent), so a listener may
    // call back into the manager without deadlocking the poll thread. The
    // listener callback is dispatched from the Cangjie side; listener
    // exceptions are contained there (this TU is built with -fno-exceptions).
    for (const auto &target : targets) {
        CWatchEvent ret = {
            .fileName = nullptr,
            .event = target.event,
            .cookie = target.cookie };
        size_t len = target.fileName.size() + 1;
        ret.fileName = (char*)malloc(len);
        if (ret.fileName == nullptr) {
            LOGE("Failed to create WatchEvent");
            continue;
        }
        int errCode = memcpy_s(ret.fileName, len, target.fileName.c_str(), len);
        if (errCode != 0) {
            LOGE("Failed to get file name");
            free(ret.fileName);
            continue;
        }
        target.watcher->watchCallback_(ret);
        free(ret.fileName);
    }
}

int FileWatcherManager::CloseNotifyFd(std::thread &finishedThread)
{
    int closeRet = SUCCESS_CODE;
    if (watcherInfoSet_.size() != 0) {
        return closeRet;
    }
    // Signal the poll thread to stop and wake it up first: close() alone does not
    // interrupt a thread blocked in poll(), so without the eventfd write the thread
    // would stay blocked forever on fd numbers that may get reused later.
    if (pollState_) {
        pollState_->stopRequested.store(true);
    }
    if (eventFd_ >= 0) {
        uint64_t wakeVal = 1;
        if (write(eventFd_, &wakeVal, sizeof(wakeVal)) < 0) {
            LOGE("Failed to wake up event fd errCode:%{public}d", errno);
        }
    }
    int notifyCloseRet = fdsan_close_with_tag(notifyFd_, CJ_FILE_FDSAN_TAG);
    if (notifyCloseRet != 0) {
        LOGE("Failed to stop notify close fd errCode:%{public}d", notifyCloseRet);
    }
    notifyFd_ = -1;
    int eventCloseRet = fdsan_close_with_tag(eventFd_, CJ_FILE_FDSAN_TAG);
    if (eventCloseRet != 0) {
        LOGE("Failed to close eventfd errCode:%{public}d", eventCloseRet);
    }
    eventFd_ = -1;
    // Every kernel watch died with the fd: clear the map so stale entries cannot
    // collide with the wd numbers of the next inotify generation.
    wdFileNameMap_.clear();
    closeRet = (notifyCloseRet != 0) ? notifyCloseRet : eventCloseRet;
    if (pollThread_.joinable()) {
        // Hand the thread over to the caller, which joins it with watchMutex_
        // released: the thread may be waiting for the mutex inside
        // ReadNotifyEvent, so joining while holding the lock would deadlock.
        // When the stop was called from inside a listener callback (which runs
        // on the poll thread itself), the caller detaches the thread instead.
        finishedThread = std::move(pollThread_);
    }
    return closeRet;
}

int32_t FileWatcherManager::StopNotify(std::shared_ptr<WatcherInfoArg> arg)
{
    std::thread finishedThread;
    int32_t rmErr = SUCCESS_CODE;
    int32_t stopCode = SUCCESS_CODE;
    {
        unique_lock<mutex> lock(watchMutex_);
        if (notifyFd_ < 0) {
            LOGE("Failed to stop notify notifyFd_:%{public}d", notifyFd_);
            // The shared fd is closed, which normally means this watcher is not
            // live any more. Still drop it from the set: a stray entry would keep
            // CloseNotifyFd disabled for every later watcher and leak the shared
            // state across users of the manager.
            RemoveWatcherInfo(arg);
            return GetErrorCode(EIO);
        }
        uint32_t newEvents = RemoveWatcherInfo(arg);
        if (newEvents > 0) {
            if (access(arg->fileName.c_str(), F_OK) == 0) {
                return NotifyToWatchNewEvents(arg->fileName, newEvents);
            }
            LOGE("The Watched file does not exist, and the remaining monitored events will be invalid.");
            return SUCCESS_CODE;
        }
        if (inotify_rm_watch(notifyFd_, arg->wd) == -1) {
            int err = errno;
            if (access(arg->fileName.c_str(), F_OK) == 0) {
                LOGE("Failed to stop notify errCode:%{public}d", err);
                rmErr = err;
            }
        }
        wdFileNameMap_.erase(arg->fileName);
        // Cleanup runs on every path reaching here, even when inotify_rm_watch
        // failed: a failed stop must not leave stale entries or an open notify
        // fd behind, otherwise later watchers leak state into each other.
        stopCode = CloseNotifyFd(finishedThread);
    }
    if (finishedThread.joinable()) {
        if (finishedThread.get_id() == std::this_thread::get_id()) {
            // The stop was called from inside a listener callback, which runs
            // on the poll thread itself: joining would deadlock. Detach
            // instead; stopRequested is already set, so the thread exits on
            // its own once the remaining listener callbacks return.
            finishedThread.detach();
        } else {
            finishedThread.join();
        }
    }
    return (rmErr != SUCCESS_CODE) ? GetErrorCode(rmErr) : stopCode;
}

bool FileWatcherManager::CheckEventValid(const uint32_t &event)
{
    if ((event & IN_ALL_EVENTS) == event) {
        return true;
    } else {
        LOGE("Param event:%{public}x is not valid", event);
        return false;
    }
}

}