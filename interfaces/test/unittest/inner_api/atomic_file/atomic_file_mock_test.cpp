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

#include "atomic_file.h"

#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "fdsan_mock.h"
#include "mmap_mock.h"
#include "stdio_mock.h"
#include "stdlib_mock.h"
#include "unistd_mock.h"
#include "ut_file_utils.h"

namespace OHOS::FileManagement::FileApi::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using ModuleFileIO::Test::FdsanMock;
using ModuleFileIO::Test::FileUtils;
using ModuleFileIO::Test::MmapMock;
using ModuleFileIO::Test::StdioMock;
using ModuleFileIO::Test::StdlibMock;
using ModuleFileIO::Test::UnistdMock;

class AtomicFileMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/InnerApiAtomicFileMockTest";
};

void AtomicFileMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "InnerApiAtomicFileMockTest");
}

void AtomicFileMockTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void AtomicFileMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void AtomicFileMockTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

static string ReadFile(const string &path)
{
    auto [succ, content] = FileUtils::ReadTextFileContent(path);
    return succ ? content : "";
}

/**
 * @tc.name: AtomicFileMockTest_FinishWrite_RenameFail_001
 * @tc.desc: Verify FinishWrite returns error when rename fails.
 */
HWTEST_F(AtomicFileMockTest, FinishWrite_RenameFail_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FinishWrite_RenameFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    stream->write("new", 3);

    auto stdioMock = StdioMock::GetMock();
    StdioMock::EnableMock();
    EXPECT_CALL(*stdioMock, rename(testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto ret = file.FinishWrite(stream);

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    StdioMock::DisableMock();
    EXPECT_FALSE(ret.IsSuccess());
}

/**
 * @tc.name: AtomicFileMockTest_DiscardWrite_RemoveFail_001
 * @tc.desc: Verify DiscardWrite returns error when remove fails with non-ENOENT errno.
 */
HWTEST_F(AtomicFileMockTest, DiscardWrite_RemoveFail_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/DiscardWrite_RemoveFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto sw = file.StartWriteByPath();
    ASSERT_TRUE(sw.IsSuccess());

    auto stdioMock = StdioMock::GetMock();
    StdioMock::EnableMock();
    EXPECT_CALL(*stdioMock, remove(testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto ret = file.DiscardWrite();

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    StdioMock::DisableMock();
    EXPECT_FALSE(ret.IsSuccess());
}

/**
 * @tc.name: AtomicFileMockTest_FailWrite_RemoveFail_001
 * @tc.desc: Verify FailWrite still returns success when remove fails inside Discard (Discard tolerates remove error).
 */
HWTEST_F(AtomicFileMockTest, FailWrite_RemoveFail_StillReturnsSuccess, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FailWrite_RemoveFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    stream->write("discard", 7);

    // Discard() internally calls remove; even if remove fails, Discard only logs a warning.
    // FailWrite always returns Success because Discard does not propagate remove errors.
    auto stdioMock = StdioMock::GetMock();
    StdioMock::EnableMock();
    EXPECT_CALL(*stdioMock, remove(testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto ret = file.FailWrite(stream);

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    StdioMock::DisableMock();
    // FailWrite returns Success regardless of remove failure in Discard
    EXPECT_TRUE(ret.IsSuccess());
    // Original file should remain intact
    EXPECT_EQ(ReadFile(path), "keep");
}

/**
 * @tc.name: AtomicFileMockTest_CommitWrite_RenameFail_001
 * @tc.desc: Verify CommitWrite returns error when rename fails after fsync.
 */
HWTEST_F(AtomicFileMockTest, CommitWrite_RenameFail_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/CommitWrite_RenameFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWriteByPath();
    ASSERT_TRUE(sw.IsSuccess());
    string tmpPath = sw.GetData().value();
    {
        ofstream ofs(tmpPath, ios::binary);
        ofs << "committed";
        ofs.flush();
        ofs.close();
    }

    auto stdioMock = StdioMock::GetMock();
    StdioMock::EnableMock();
    EXPECT_CALL(*stdioMock, rename(testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EACCES, -1));

    auto ret = file.CommitWrite();

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    StdioMock::DisableMock();
    EXPECT_FALSE(ret.IsSuccess());
    // Original file should remain intact
    EXPECT_EQ(ReadFile(path), "old");
}

/**
 * @tc.name: AtomicFileMockTest_FinishWrite_WriteFail_001
 * @tc.desc: Verify FinishWrite handles write failure gracefully (stream flush fails).
 */
HWTEST_F(AtomicFileMockTest, FinishWrite_WriteFail_HandlesGracefully, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FinishWrite_WriteFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();

    // Mock write to fail, causing flush to fail; mock fdsan close to avoid tag issues
    auto unistdMock = UnistdMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    UnistdMock::EnableMock();
    FdsanMock::EnableMock();
    EXPECT_CALL(*unistdMock, write(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::SetErrnoAndReturn(EIO, -1));
    EXPECT_CALL(*unistdMock, close(testing::_))
        .WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_))
        .WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*fdsanMock, fdsan_get_owner_tag(testing::_))
        .WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*fdsanMock, fdsan_exchange_owner_tag(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return());

    string content = "data that will fail to flush";
    stream->write(content.data(), content.size());
    auto ret = file.FinishWrite(stream);

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    UnistdMock::DisableMock();
    FdsanMock::DisableMock();

    // flush should have failed, resulting in Discard and error
    EXPECT_FALSE(ret.IsSuccess());
    // Original file should remain intact
    EXPECT_EQ(ReadFile(path), "old");
}

/**
 * @tc.name: AtomicFileMockTest_DiscardWrite_EnoentTolerated_001
 * @tc.desc: Verify DiscardWrite tolerates ENOENT from remove (file already gone).
 */
HWTEST_F(AtomicFileMockTest, DiscardWrite_EnoentTolerated_ReturnsSuccess, testing::ext::TestSize.Level1)
{
    string path = testDir + "/DiscardWrite_Enoent.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto sw = file.StartWriteByPath();
    ASSERT_TRUE(sw.IsSuccess());

    // Remove the temp file first
    string tmpPath = sw.GetData().value();
    ASSERT_TRUE(FileUtils::RemoveAll(tmpPath));

    // Now mock remove to return ENOENT (simulating race or double-cleanup)
    auto stdioMock = StdioMock::GetMock();
    StdioMock::EnableMock();
    EXPECT_CALL(*stdioMock, remove(testing::_))
        .WillOnce(testing::SetErrnoAndReturn(ENOENT, -1));

    auto ret = file.DiscardWrite();

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    StdioMock::DisableMock();
    EXPECT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ReadFile(path), "keep");
}

/**
 * @tc.name: AtomicFileMockTest_FailWrite_RemoveFailNonEnoent_001
 * @tc.desc: Verify Discard tolerates a non-ENOENT remove failure (only warns) inside FailWrite.
 */
HWTEST_F(AtomicFileMockTest, FailWrite_DiscardRemoveFailNonEnoent_StillReturnsSuccess, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FailWrite_DiscardRemoveFailNonEnoent.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    stream->write("discard", 7);

    // remove fails with EACCES (non-ENOENT): Discard logs a warning but does not propagate
    auto stdioMock = StdioMock::GetMock();
    StdioMock::EnableMock();
    EXPECT_CALL(*stdioMock, remove(testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EACCES, -1));

    auto ret = file.FailWrite(stream);

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    StdioMock::DisableMock();
    EXPECT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ReadFile(path), "keep");
}

/**
 * @tc.name: AtomicFileMockTest_ReadFully_FstatFail_001
 * @tc.desc: Verify ReadFully returns error when fstat fails.
 */
HWTEST_F(AtomicFileMockTest, ReadFully_FstatFail_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/ReadFully_FstatFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "data"));

    AtomicFile file(path);
    auto mmapMock = MmapMock::GetMock();
    MmapMock::EnableMock();
    EXPECT_CALL(*mmapMock, fstat(testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EBADF, -1));

    string out;
    auto ret = file.ReadFully(out);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    MmapMock::DisableMock();
    EXPECT_FALSE(ret.IsSuccess());
    EXPECT_EQ(ret.GetError().GetErrNo(), 13900008); // EBADF
    EXPECT_TRUE(out.empty());
}

/**
 * @tc.name: AtomicFileMockTest_ReadFully_ReadEintrThenFail_001
 * @tc.desc: Verify ReadFully retries read on EINTR and fails on a subsequent EIO.
 */
HWTEST_F(AtomicFileMockTest, ReadFully_ReadEintrThenEio_ReturnsEioError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/ReadFully_ReadEintrThenEio.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    AtomicFile file(path);
    auto unistdMock = UnistdMock::GetMock();
    UnistdMock::EnableMock();
    // First read is interrupted (EINTR triggers the retry loop), second read fails with EIO
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EINTR, -1))
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    string out;
    auto ret = file.ReadFully(out);

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    UnistdMock::DisableMock();
    EXPECT_FALSE(ret.IsSuccess());
    EXPECT_EQ(ret.GetError().GetErrNo(), 13900005); // EIO
    // Note: out's content is unspecified on failure (ReadFully resizes it before reading);
    // callers must ignore out when ReadFully fails.
}

/**
 * @tc.name: AtomicFileMockTest_ReadFully_ReadReturnsZero_001
 * @tc.desc: Verify ReadFully succeeds with empty output when read returns 0 (early EOF).
 */
HWTEST_F(AtomicFileMockTest, ReadFully_ReadReturnsZero_ReturnsSuccessWithEmptyOutput, testing::ext::TestSize.Level1)
{
    string path = testDir + "/ReadFully_ReadReturnsZero.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    AtomicFile file(path);
    auto unistdMock = UnistdMock::GetMock();
    UnistdMock::EnableMock();
    // read returns 0: treated as early EOF, loop breaks and out is resized to 0
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(0));

    string out;
    auto ret = file.ReadFully(out);

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    UnistdMock::DisableMock();
    EXPECT_TRUE(ret.IsSuccess());
    EXPECT_TRUE(out.empty());
}

/**
 * @tc.name: AtomicFileMockTest_StartWrite_MkstempFail_001
 * @tc.desc: Verify StartWrite returns error when mkstemp fails.
 */
HWTEST_F(AtomicFileMockTest, StartWrite_MkstempFail_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWrite_MkstempFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto stdlibMock = StdlibMock::GetMock();
    StdlibMock::EnableMock();
    EXPECT_CALL(*stdlibMock, mkstemp(testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EACCES, -1));

    auto sw = file.StartWrite();

    testing::Mock::VerifyAndClearExpectations(stdlibMock.get());
    StdlibMock::DisableMock();
    EXPECT_FALSE(sw.IsSuccess());
    EXPECT_EQ(sw.GetError().GetErrNo(), 13900012); // EACCES
    // No temp file should exist at the unexpanded _XXXXXX path
    EXPECT_FALSE(FileUtils::Exists(file.GetTmpPath()));
    EXPECT_EQ(ReadFile(path), "keep");
}

/**
 * @tc.name: AtomicFileMockTest_StartWrite_SyncParentDirFail_001
 * @tc.desc: Verify StartWrite cleans up the temp file when SyncParentDir fails after mkstemp.
 */
HWTEST_F(AtomicFileMockTest, StartWrite_SyncParentDirFail_CleansUpTempFile, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWrite_SyncParentDirFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto stdlibMock = StdlibMock::GetMock();
    StdlibMock::EnableMock();
    // mkstemp passes through to the real implementation; fsync fails inside SyncParentDir
    EXPECT_CALL(*stdlibMock, mkstemp(testing::_))
        .WillOnce(testing::Invoke([](char *templateName) {
            return StdlibMock::InvokeRealMkstemp(templateName);
        }));
    EXPECT_CALL(*stdlibMock, fsync(testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto sw = file.StartWrite();

    testing::Mock::VerifyAndClearExpectations(stdlibMock.get());
    StdlibMock::DisableMock();
    EXPECT_FALSE(sw.IsSuccess());
    EXPECT_EQ(sw.GetError().GetErrNo(), 13900005); // EIO
    // CreateTempFile must have closed the fd and removed the mkstemp temp file
    EXPECT_FALSE(FileUtils::Exists(file.GetTmpPath()));
    EXPECT_EQ(ReadFile(path), "old");
}

/**
 * @tc.name: AtomicFileMockTest_SyncParentDir_FsyncFail_001
 * @tc.desc: Verify SyncParentDir returns error when fsync on the parent dir fails.
 */
HWTEST_F(AtomicFileMockTest, SyncParentDir_FsyncFail_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/SyncParentDir_FsyncFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto stdlibMock = StdlibMock::GetMock();
    StdlibMock::EnableMock();
    EXPECT_CALL(*stdlibMock, fsync(testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto ret = file.SyncParentDir();

    testing::Mock::VerifyAndClearExpectations(stdlibMock.get());
    StdlibMock::DisableMock();
    EXPECT_FALSE(ret.IsSuccess());
    EXPECT_EQ(ret.GetError().GetErrNo(), 13900005); // EIO
}

/**
 * @tc.name: AtomicFileMockTest_FinishWrite_FsyncFail_001
 * @tc.desc: Verify FinishWrite discards the stream and returns error when fsync of data fails.
 */
HWTEST_F(AtomicFileMockTest, FinishWrite_FsyncFail_DiscardsAndReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FinishWrite_FsyncFail.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    // StartWrite with mocks disabled: real mkstemp/fsync path succeeds
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    stream->write("new", 3);

    // Data fsync fails inside FinishWrite: stream must be discarded
    auto stdlibMock = StdlibMock::GetMock();
    StdlibMock::EnableMock();
    EXPECT_CALL(*stdlibMock, fsync(testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto ret = file.FinishWrite(stream);

    testing::Mock::VerifyAndClearExpectations(stdlibMock.get());
    StdlibMock::DisableMock();
    EXPECT_FALSE(ret.IsSuccess());
    EXPECT_EQ(ret.GetError().GetErrNo(), 13900005); // EIO
    // Discard removed the temp file and the original content is intact
    EXPECT_FALSE(FileUtils::Exists(file.GetTmpPath()));
    EXPECT_EQ(ReadFile(path), "old");
}
} // namespace OHOS::FileManagement::FileApi::Test
