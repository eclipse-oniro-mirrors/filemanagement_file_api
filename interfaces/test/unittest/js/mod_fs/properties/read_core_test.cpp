/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "read_core.h"

#include <cstdint>
#include <climits>
#include <gtest/gtest.h>


namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class ReadCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ReadCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void ReadCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void ReadCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ReadCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ReadCoreTest_DoRead_001
 * @tc.desc: Test function of ReadCore::DoRead interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadCoreTest, ReadCoreTest_DoRead_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadCoreTest-begin ReadCoreTest_DoRead_001";

    int32_t fd = -1;
    void *buf = nullptr;
    ArrayBuffer arrayBuffer(buf, 0);

    auto res = ReadCore::DoRead(fd, arrayBuffer);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "ReadCoreTest-end ReadCoreTest_DoRead_001";
}

/**
 * @tc.name: ReadCoreTest_DoRead_002
 * @tc.desc: Test function of ReadCore::DoRead interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadCoreTest, ReadCoreTest_DoRead_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadCoreTest-begin ReadCoreTest_DoRead_002";

    int32_t fd = 1;
    void *buf = nullptr;
    ArrayBuffer arrayBuffer(buf, 0xffffffff + 1);

    auto res = ReadCore::DoRead(fd, arrayBuffer);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "ReadCoreTest-end ReadCoreTest_DoRead_002";
}

/**
 * @tc.name: ReadCoreTest_DoRead_003
 * @tc.desc: Test function of ReadCore::DoRead interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadCoreTest, ReadCoreTest_DoRead_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadCoreTest-begin ReadCoreTest_DoRead_003";

    int32_t fd = 1;
    void *buf = nullptr;
    ArrayBuffer arrayBuffer(buf, 0);
    optional<ReadOptions> options = std::make_optional<ReadOptions>();
    options->offset = std::make_optional<int64_t>(-1);

    auto res = ReadCore::DoRead(fd, arrayBuffer, options);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "ReadCoreTest-end ReadCoreTest_DoRead_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test