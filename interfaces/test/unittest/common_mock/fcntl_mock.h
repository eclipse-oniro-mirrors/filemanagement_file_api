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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_FCNTL_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_FCNTL_MOCK_H

#include <fcntl.h>

#include <gmock/gmock.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

class IFcntlMock {
public:
    virtual ~IFcntlMock() = default;
    virtual int fcntl(int, int) = 0;
};

class FcntlMock : public IFcntlMock {
public:
    MOCK_METHOD(int, fcntl, (int, int), (override));

public:
    static std::shared_ptr<FcntlMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<FcntlMock> fcntlMock;
    static thread_local bool mockable;
};

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_FCNTL_MOCK_H
