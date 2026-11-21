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

#include "fs_atomicfile.h"

#include <cerrno>
#include <cstring>
#include <dlfcn.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/stat.h>

#include "fdsan_mock.h"
#include "mmap_mock.h"
#include "stdio_mock.h"
#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsAtomicFileMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/FsAtomicFileMockTest";
};

void FsAtomicFileMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsAtomicFileMockTest");
    StdioMock::EnableMock();
}

void FsAtomicFileMockTest::TearDownTestSuite()
{
    StdioMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsAtomicFileMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void FsAtomicFileMockTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsAtomicFileMockTest_FailWrite_001
 * @tc.desc: Test function of FsAtomicFile::FailWrite interface for FAILURE when remove fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */

HWTEST_F(FsAtomicFileMockTest, FsAtomicFileMockTest_FailWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicFileMockTest-begin FsAtomicFileMockTest_FailWrite_001";

    string basePath = testDir + "/FsAtomicFileMockTest_FailWrite_001.txt";
    auto ctorRet = FsAtomicFile::Constructor(basePath);
    ASSERT_TRUE(ctorRet.IsSuccess());
    std::unique_ptr<FsAtomicFile> file(ctorRet.GetData().value());

    auto swRet = file->StartWrite();
    ASSERT_TRUE(swRet.IsSuccess());

    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*stdioMock, remove(testing::_)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto ret = file->FailWrite();

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsAtomicFileMockTest-end FsAtomicFileMockTest_FailWrite_001";
}

/**
 * @tc.name: FsAtomicFileMockTest_ReadFully_001
 * @tc.desc: Test function of FsAtomicFile::ReadFully interface for FAILURE when memcpy_s fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsAtomicFileMockTest, FsAtomicFileMockTest_ReadFully_MemcpyFail, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicFileMockTest-begin FsAtomicFileMockTest_ReadFully_MemcpyFail";

    string basePath = testDir + "/FsAtomicFileMockTest_ReadFully_MemcpyFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(basePath, "content"));

    auto ctorRet = FsAtomicFile::Constructor(basePath);
    ASSERT_TRUE(ctorRet.IsSuccess());
    std::unique_ptr<FsAtomicFile> file(ctorRet.GetData().value());
    ASSERT_NE(file, nullptr);

    auto mmapMock = MmapMock::GetMock();
    MmapMock::EnableMock();
    // While MmapMock is enabled, fstat is intercepted too: report the real file size
    // so ReadFully reaches the memcpy_s call.
    EXPECT_CALL(*mmapMock, fstat(testing::_, testing::_))
        .WillOnce(testing::Invoke([](int, struct stat *statbuf) {
            memset(statbuf, 0, sizeof(*statbuf));
            statbuf->st_size = 7; // "content".size()
            return 0;
        }));
    // While MmapMock is enabled, memcpy_s is intercepted process-wide; unrelated callers
    // (std::string/hilog internals) must still get the real copy, otherwise memory gets
    // corrupted. Pass unmatched calls through to the real memcpy_s.
    ON_CALL(*mmapMock, memcpy_s(testing::_, testing::_, testing::_, testing::_))
        .WillByDefault(testing::Invoke([](void *dest, size_t destMax, const void *src, size_t count) -> errno_t {
            static auto realMemcpyS = reinterpret_cast<errno_t (*)(void *, size_t, const void *, size_t)>(
                dlsym(RTLD_NEXT, "memcpy_s"));
            return realMemcpyS(dest, destMax, src, count);
        }));
    // Only the target buffer copy (destMax == count == content size) fails with ERANGE:
    // ReadFully must free the buffer and return EIO. Use ON_CALL (gmock picks the last
    // matching default action) instead of EXPECT_CALL: after saturation, later unrelated
    // memcpy_s calls would be reported as unexpected-call failures by gmock.
    ON_CALL(*mmapMock, memcpy_s(testing::_, 7, testing::_, 7))
        .WillByDefault(testing::Return(ERANGE));

    auto ret = file->ReadFully();

    // VerifyAndClear (not VerifyAndClearExpectations) also removes the ON_CALL default
    // actions, so the pass-through/ERANGE stubs do not leak into later tests.
    testing::Mock::VerifyAndClear(mmapMock.get());
    MmapMock::DisableMock();
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsAtomicFileMockTest-end FsAtomicFileMockTest_ReadFully_MemcpyFail";
}
}
