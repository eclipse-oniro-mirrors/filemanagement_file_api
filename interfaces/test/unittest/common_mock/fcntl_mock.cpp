/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "fcntl_mock.h"

#include <cstdarg>
#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<FcntlMock> FcntlMock::fcntlMock = nullptr;
thread_local bool FcntlMock::mockable = false;

std::shared_ptr<FcntlMock> FcntlMock::GetMock()
{
    if (fcntlMock == nullptr) {
        fcntlMock = std::make_shared<FcntlMock>();
    }

    return fcntlMock;
}

void FcntlMock::EnableMock()
{
    mockable = true;
}

void FcntlMock::DisableMock()
{
    fcntlMock = nullptr;
    mockable = false;
}

bool FcntlMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using FcntlMock = OHOS::FileManagement::ModuleFileIO::Test::FcntlMock;

int fcntl(int fd, int cmd, ...)
{
    if (FcntlMock::IsMockable()) {
        return FcntlMock::GetMock()->fcntl(fd, cmd);
    }

    static int (*realFcntl)(int, int, ...) = []() {
        auto func = (int (*)(int, int, ...))dlsym(RTLD_NEXT, "fcntl");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fcntl: " << dlerror();
        }
        return func;
    }();

    if (!realFcntl) {
        return -1;
    }

    va_list ap;
    va_start(ap, cmd);
    int ret;
    switch (cmd) {
        /* cmd without the third argument */
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:
            ret = realFcntl(fd, cmd);
            break;
        /* cmd with an int third argument */
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETFL:
        case F_SETOWN:
            ret = realFcntl(fd, cmd, va_arg(ap, int));
            break;
        /* others, e.g. F_GETLK/F_SETLK/F_SETLKW with a struct flock * third argument */
        default:
            ret = realFcntl(fd, cmd, va_arg(ap, void *));
            break;
    }
    va_end(ap);
    return ret;
}
} // extern "C"
#endif
