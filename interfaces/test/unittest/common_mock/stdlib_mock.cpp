/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stdlib_mock.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

thread_local std::shared_ptr<StdlibMock> StdlibMock::stdlibMock = nullptr;
thread_local bool StdlibMock::mockable = false;

std::shared_ptr<StdlibMock> StdlibMock::GetMock()
{
    if (stdlibMock == nullptr) {
        stdlibMock = std::make_shared<StdlibMock>();
    }
    return stdlibMock;
}

void StdlibMock::EnableMock()
{
    mockable = true;
}

void StdlibMock::DisableMock()
{
    stdlibMock = nullptr;
    mockable = false;
}

bool StdlibMock::IsMockable()
{
    return mockable;
}

int StdlibMock::InvokeRealMkstemp(char *templateName)
{
    static int (*realMkstemp)(char *) = []() {
        auto func = (int (*)(char *))dlsym(RTLD_NEXT, "mkstemp");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real mkstemp: " << dlerror();
        }
        return func;
    }();
    if (!realMkstemp) {
        return -1;
    }
    return realMkstemp(templateName);
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

#ifdef __cplusplus
extern "C" {
using StdlibMock = OHOS::FileManagement::ModuleFileIO::Test::StdlibMock;

int mkstemp(char *templateName)
{
    if (StdlibMock::IsMockable()) {
        return StdlibMock::GetMock()->mkstemp(templateName);
    }

    static int (*realMkstemp)(char *) = []() {
        auto func = (int (*)(char *))dlsym(RTLD_NEXT, "mkstemp");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real mkstemp: " << dlerror();
        }
        return func;
    }();

    if (!realMkstemp) {
        return -1;
    }

    return realMkstemp(templateName);
}

// Note: fsync is intercepted here instead of the shared unistd_mock to avoid
// changing behavior of other test targets that enable UnistdMock but rely on
// the real fsync. This file is only linked into atomic_file_mock_test.
int fsync(int fd)
{
    if (StdlibMock::IsMockable()) {
        return StdlibMock::GetMock()->fsync(fd);
    }

    static int (*realFsync)(int) = []() {
        auto func = (int (*)(int))dlsym(RTLD_NEXT, "fsync");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fsync: " << dlerror();
        }
        return func;
    }();

    if (!realFsync) {
        return -1;
    }

    return realFsync(fd);
}

} // extern "C"
#endif
