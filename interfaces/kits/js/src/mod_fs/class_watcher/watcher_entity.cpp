/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "watcher_entity.h"

#include <cerrno>
#include <unistd.h>

#include "filemgmt_libhilog.h"
#include "uv.h"
namespace OHOS::FileManagement::ModuleFileIO {
using namespace OHOS::FileManagement::LibN;
FileWatcher::FileWatcher() {}

FileWatcher::~FileWatcher() {}

bool FileWatcher::InitNotify(int &fd)
{
    fd = inotify_init();
    if (fd < 0) {
        HILOGE("Failed to init notify fail errCode:%{public}d", errno);
        return false;
    }
    return true;
}

bool FileWatcher::StartNotify(WatcherInfoArg &arg)
{
    int wd = inotify_add_watch(arg.fd, arg.filename.c_str(), arg.events);
    if (wd < 0) {
        HILOGE("Failed to start notify fail errCode:%{public}d", errno);
        return false;
    }
    arg.wd = wd;
    run_ = true;
    return true;
}

bool FileWatcher::StopNotify(const WatcherInfoArg &arg)
{
    run_ = false;
    if (arg.events == IN_DELETE_SELF) {
        close(arg.fd);
        return true;
    }
    if (inotify_rm_watch(arg.fd, arg.wd) == -1) {
        HILOGE("Failed to stop notify fail errCode:%{public}d", errno);
        return false;
    }
    close(arg.fd);
    return true;
}

void FileWatcher::HandleEvent(WatcherInfoArg &arg, const struct inotify_event *event, WatcherCallback callback)
{
    if (event->wd != arg.wd) {
        return;
    }
    std::string fileName = arg.filename;
    if (event->len > 0) {
        fileName.append(std::string("/"));
        fileName.append(std::string(event->name));
    }
    callback(arg.env, arg.nRef, fileName, event->mask, event->cookie);
}

void FileWatcher::GetNotifyEvent(WatcherInfoArg &arg, WatcherCallback callback)
{
    char buf[BUF_SIZE] = {0};
    struct inotify_event *event = nullptr;
    while (run_) {
        int fd = arg.fd;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        if (select(fd + 1, &fds, nullptr, nullptr, nullptr) > 0) {
            int len, index = 0;
            while (((len = read(fd, &buf, sizeof(buf))) < 0) && (errno == EINTR)) {};
            while (index < len) {
                event = reinterpret_cast<inotify_event*>(buf + index);
                HandleEvent(arg, event, callback);
                index += sizeof(struct inotify_event) + event->len;
            }
        }
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO