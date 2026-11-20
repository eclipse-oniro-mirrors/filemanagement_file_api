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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_STDLIB_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_STDLIB_MOCK_H

#include <stdlib.h>
#include <unistd.h>

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

class IStdlibMock {
public:
    virtual ~IStdlibMock() = default;
    virtual int mkstemp(char *) = 0;
    virtual int fsync(int) = 0;
};

class StdlibMock : public IStdlibMock {
public:
    MOCK_METHOD(int, mkstemp, (char *), (override));
    MOCK_METHOD(int, fsync, (int), (override));

public:
    static std::shared_ptr<StdlibMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();
    // Invokes the real (dlsym-resolved) mkstemp; for mock actions that need to
    // keep real behavior for one symbol while mocking another.
    static int InvokeRealMkstemp(char *templateName);

private:
    static thread_local std::shared_ptr<StdlibMock> stdlibMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_STDLIB_MOCK_H
