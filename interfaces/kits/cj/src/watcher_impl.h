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

#ifndef OHOS_FILE_FS_WATCHER_IMPL_H
#define OHOS_FILE_FS_WATCHER_IMPL_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <sys/inotify.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "uni_error.h"
#include "ffi_remote_data.h"
#include "cj_common_ffi.h"
#include "cj_lambda.h"
#include "singleton.h"

namespace OHOS {
namespace CJSystemapi {
constexpr int BUF_SIZE = 1024;

struct CWatchEvent {
    char* fileName;
    uint32_t event;
    uint32_t cookie;
};
struct WatcherInfoArg {
    std::string fileName = "";
    uint32_t events = 0;
    int wd = -1;
    // Whether the watch target resolved to a directory at start() time. Only a
    // directory watch semantically receives child events that carry a name; see
    // the name-append logic in NotifyEvent.
    bool isDir = false;
    int64_t callbackId = -1;
    std::function<void(CWatchEvent)> watchCallback_;
    explicit WatcherInfoArg(void (*callback)(CWatchEvent))
    {
        watchCallback_ = CJLambda::Create(callback);
        callbackId = reinterpret_cast<int64_t>(callback);
    }
    ~WatcherInfoArg() = default;
};

class WatcherImpl : public OHOS::FFI::FFIData {
    DECL_TYPE(WatcherImpl, OHOS::FFI::FFIData)
public:
    std::shared_ptr<WatcherInfoArg> data_;
};

class FileWatcherManager : public Singleton<FileWatcherManager> {
public:
    FileWatcherManager();
    ~FileWatcherManager();
    FileWatcherManager(FileWatcherManager const &) = delete;
    void operator=(FileWatcherManager const &) = delete;

    int32_t GetNotifyId();
    bool InitNotify();
    bool AddWatcherInfo(const std::string &fileName, std::shared_ptr<WatcherInfoArg> arg);
    bool CheckEventValid(const uint32_t &event);
    int32_t StartNotify(std::shared_ptr<WatcherInfoArg> arg);
    void GetNotifyEvent();
    void ReadNotifyEvent(int32_t notifyFd);
    int32_t StopNotify(std::shared_ptr<WatcherInfoArg> arg);

private:
    // Per poll-thread generation state: every spawned thread owns its own
    // instance, so stopping one generation can never clobber the flag of a
    // newer generation started while the old thread is still exiting.
    struct PollState {
        std::atomic<bool> stopRequested { false };
        std::atomic<bool> exited { false };
    };
    // One deferred listener notification: built under watchMutex_ and
    // dispatched after the mutex is released. The shared_ptr keeps the watcher
    // (and its callback) alive, so dispatching outside the lock cannot touch
    // freed state.
    struct NotifyTarget {
        std::shared_ptr<WatcherInfoArg> watcher;
        std::string fileName;
        uint32_t event = 0;
        uint32_t cookie = 0;
    };
    uint32_t RemoveWatcherInfo(std::shared_ptr<WatcherInfoArg> arg);
    // Appends the watchers matching one inotify event, with their resolved
    // paths, to targets. Called with watchMutex_ held (see ReadNotifyEvent);
    // takes no lock itself.
    void NotifyEvent(const struct inotify_event *event, std::vector<NotifyTarget> &targets);
    // Invokes the deferred listener callbacks with watchMutex_ released, so a
    // listener that calls stop()/start()/createWatcher() cannot deadlock the
    // poll thread.
    void DispatchNotifyEvents(const std::vector<NotifyTarget> &targets);
    // Body of one poll-thread generation: blocks on the eventfd/inotify pair
    // until stopRequested is set or the fds turn invalid, reading and
    // dispatching each inotify wakeup. Runs entirely on the spawned thread;
    // stores exited as its last statement.
    void PollNotifyLoop(std::shared_ptr<PollState> state, int32_t notifyFd, int32_t eventFd);
    int CloseNotifyFd(std::thread &finishedThread);
    int NotifyToWatchNewEvents(const std::string &fileName, const uint32_t &watchEvents);
    // The kernel never hands out the wd of a still-live watch to a new
    // inotify_add_watch, so any OTHER map entry still recording this wd can only
    // be a stale leftover of a watch that already died (auto-removed when its
    // inode was deleted). Purging those entries keeps a recycled wd owned by
    // exactly one path and, unlike an IN_IGNORED-driven purge, is race free: it
    // runs synchronously under watchMutex_ together with the map write.
    void RemoveStaleWdEntries(const std::string &fileName, int wd);

private:
    static std::mutex watchMutex_;
    std::shared_ptr<PollState> pollState_;
    std::thread pollThread_;
    int32_t notifyFd_ = -1;
    int32_t eventFd_ = -1;
    std::unordered_set<std::shared_ptr<WatcherInfoArg>> watcherInfoSet_;
    std::unordered_map<std::string, std::pair<int, uint32_t>> wdFileNameMap_;
};
} // OHOS::FileManagement::ModuleFileIO
}

#endif // OHOS_FILE_FS_IMPL_H