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

#include "datashare_mock.h"

#include <dlfcn.h>

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

thread_local std::shared_ptr<DataShareMock> DataShareMock::dataShareMock = nullptr;
thread_local bool DataShareMock::mockable = false;

std::shared_ptr<DataShareMock> DataShareMock::GetMock()
{
    if (dataShareMock == nullptr) {
        dataShareMock = std::make_shared<DataShareMock>();
    }
    return dataShareMock;
}

void DataShareMock::EnableMock()
{
    mockable = true;
}

void DataShareMock::DisableMock()
{
    dataShareMock = nullptr;
    mockable = false;
}

bool DataShareMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

namespace OHOS::DataShare {
using DataShareMock = OHOS::FileManagement::ModuleFileIO::Test::DataShareMock;

// Link-time override of the real Creator in libdatashare_consumer.z.so: this definition wins for every
// in-image caller; when the mock is disabled, RTLD_NEXT falls back to the real implementation.
// Adjacent string literals below concatenate into a single constexpr name.
static constexpr char REAL_CREATOR_NAME[] =
    "_ZN4OHOS9DataShare15DataShareHelper7CreatorERKNS_4sptrINS_13IRemoteObjectEEERKNSt3__h12"
    "basic_stringIcNS7_11char_traitsIcEENS7_9allocatorIcEEEESF_ib";

std::shared_ptr<DataShareHelper> DataShareHelper::Creator(const sptr<IRemoteObject> &token,
    const std::string &strUri, const std::string &extUri, const int waitTime, bool isSystem)
{
    if (DataShareMock::IsMockable()) {
        return DataShareMock::GetMock()->Creator(token, strUri);
    }

    // REAL_CREATOR_NAME is the Itanium mangled name of
    //   std::shared_ptr<DataShareHelper> DataShareHelper::Creator(const sptr<IRemoteObject> &,
    //       const std::string &, const std::string &, const int, bool)
    // in libdatashare_consumer.z.so (verified with c++filt). It embeds the libc++ inline
    // namespace of the OHOS toolchain (`St3__h1`), so it must be re-verified on toolchain
    // upgrades; a mismatch would make dlsym fail below and fail the test loudly instead of
    // silently degrading every non-mocked case to the Creator-returns-nullptr branch.
    static std::shared_ptr<DataShareHelper> (*realCreator)(const sptr<IRemoteObject> &,
        const std::string &, const std::string &, const int, bool) = []() {
        auto func = (std::shared_ptr<DataShareHelper> (*)(const sptr<IRemoteObject> &,
            const std::string &, const std::string &, const int, bool))dlsym(RTLD_NEXT,
            REAL_CREATOR_NAME);
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real DataShareHelper::Creator: " << dlerror();
            ADD_FAILURE() << "Real DataShareHelper::Creator unresolved; non-mocked cases would "
                             "silently degrade to the nullptr branch";
        }
        return func;
    }();

    if (!realCreator) {
        return nullptr;
    }
    return realCreator(token, strUri, extUri, waitTime, isSystem);
}
} // namespace OHOS::DataShare
