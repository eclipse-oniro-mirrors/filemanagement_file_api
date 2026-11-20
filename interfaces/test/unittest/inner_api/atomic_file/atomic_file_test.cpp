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

#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <limits.h>
#include <sys/prctl.h>
#include <type_traits>
#include <unistd.h>

#include "atomic_write_stream.h"

#include "ut_file_utils.h"

// Compile-time verification: AtomicFile is neither copy-constructible nor copy-assignable.
static_assert(!std::is_copy_constructible_v<OHOS::FileManagement::FileApi::AtomicFile>,
    "AtomicFile must not be copy-constructible");
static_assert(!std::is_copy_assignable_v<OHOS::FileManagement::FileApi::AtomicFile>,
    "AtomicFile must not be copy-assignable");
static_assert(std::is_move_constructible_v<OHOS::FileManagement::FileApi::AtomicFile>,
    "AtomicFile must be move-constructible");
static_assert(std::is_move_assignable_v<OHOS::FileManagement::FileApi::AtomicFile>,
    "AtomicFile must be move-assignable");

namespace OHOS::FileManagement::FileApi::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using ModuleFileIO::Test::FileUtils;
namespace fs = std::filesystem;

class AtomicFileTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/InnerApiAtomicFileTest";
};

void AtomicFileTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "InnerApiAtomicFileTest");
}

void AtomicFileTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void AtomicFileTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void AtomicFileTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

static string ReadFile(const string &path)
{
    auto [succ, content] = FileUtils::ReadTextFileContent(path);
    return succ ? content : "";
}

// Read file in binary mode so embedded '\0' bytes are preserved (ReadTextFileContent
// uses text-mode ifstream which truncates at NUL). Returns empty string on failure.
static string ReadFileBinary(const string &path)
{
    ifstream ifs(path, ios::binary);
    if (!ifs.is_open()) {
        return "";
    }
    ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

/**
 * @tc.name: AtomicFileTest_GetPath_001
 * @tc.desc: Verify GetPath returns basePath and GetTmpPath returns basePath+suffix.
 */
HWTEST_F(AtomicFileTest, GetPath_ReturnsBasePathAndTmpPath, testing::ext::TestSize.Level1)
{
    string path = testDir + "/GetPath_001.txt";
    AtomicFile file(path);
    EXPECT_EQ(file.GetPath(), path);
    EXPECT_EQ(file.GetTmpPath(), path + "_XXXXXX");
}

/**
 * @tc.name: AtomicFileTest_StartWriteFinishWrite_001
 * @tc.desc: Verify StartWrite returns ostream and FinishWrite replaces file content with fsync.
 */
HWTEST_F(AtomicFileTest, StartWriteFinishWrite_ReplacesContent, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWriteFinishWrite_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    ASSERT_NE(stream, nullptr);
    string content = "hello atomic";
    stream->write(content.data(), content.size());
    auto fw = file.FinishWrite(stream);
    EXPECT_TRUE(fw.IsSuccess());
    EXPECT_EQ(ReadFile(path), content);
}

/**
 * @tc.name: AtomicFileTest_StartWriteFailWrite_001
 * @tc.desc: Verify FailWrite discards temp file and keeps original content.
 */
HWTEST_F(AtomicFileTest, FailWrite_KeepsOriginalContent, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWriteFailWrite_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "original"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    stream->write("discard", 7);
    auto fw = file.FailWrite(stream);
    EXPECT_TRUE(fw.IsSuccess());
    EXPECT_EQ(ReadFile(path), "original");
}

/**
 * @tc.name: AtomicFileTest_OstreamDestructor_001
 * @tc.desc: Verify ostream destructor triggers FailWrite when forgotten by caller.
 */
HWTEST_F(AtomicFileTest, OstreamDestructor_TriggersFailWrite, testing::ext::TestSize.Level1)
{
    string path = testDir + "/OstreamDestructor_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    string tmpPath;
    {
        AtomicFile file(path);
        auto sw = file.StartWrite();
        ASSERT_TRUE(sw.IsSuccess());
        auto stream = sw.GetData().value();
        stream->write("forgotten", 9);
        tmpPath = file.GetTmpPath();
    }
    EXPECT_FALSE(FileUtils::Exists(tmpPath));
    EXPECT_EQ(ReadFile(path), "keep");
}

/**
 * @tc.name: AtomicFileTest_StartWriteByPathCommitWrite_001
 * @tc.desc: Verify path-based StartWriteByPath/CommitWrite with explicit file close.
 */
HWTEST_F(AtomicFileTest, StartWriteByPathCommitWrite_ReplacesContent, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWriteByPathCommitWrite_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWriteByPath();
    ASSERT_TRUE(sw.IsSuccess());
    string tmpPath = sw.GetData().value();
    EXPECT_TRUE(FileUtils::Exists(tmpPath));
    {
        ofstream ofs(tmpPath, ios::binary);
        ofs << "committed";
        ofs.flush();
        ofs.close();
    }
    EXPECT_FALSE(FileUtils::Exists(tmpPath + ".stale"));
    auto cw = file.CommitWrite();
    EXPECT_TRUE(cw.IsSuccess());
    EXPECT_EQ(ReadFile(path), "committed");
}

/**
 * @tc.name: AtomicFileTest_StartWriteByPathDiscardWrite_001
 * @tc.desc: Verify DiscardWrite removes temp file and keeps original.
 */
HWTEST_F(AtomicFileTest, DiscardWrite_RemovesTempFile, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWriteByPathDiscardWrite_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto sw = file.StartWriteByPath();
    ASSERT_TRUE(sw.IsSuccess());
    string tmpPath = sw.GetData().value();
    auto dw = file.DiscardWrite();
    EXPECT_TRUE(dw.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(tmpPath));
    EXPECT_EQ(ReadFile(path), "keep");
}

/**
 * @tc.name: AtomicFileTest_StartWrite_001
 * @tc.desc: Verify StartWrite returns ENOENT when parent directory does not exist.
 */
HWTEST_F(AtomicFileTest, StartWrite_NonExistentParentDir_ReturnsEnoent, testing::ext::TestSize.Level1)
{
    string path = testDir + "/nonexistent_subdir/file.txt";
    AtomicFile file(path);
    auto sw = file.StartWrite();
    EXPECT_FALSE(sw.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_StartWriteByPath_001
 * @tc.desc: Verify StartWriteByPath returns ENOENT when parent directory does not exist.
 */
HWTEST_F(AtomicFileTest, StartWriteByPath_NonExistentParentDir_ReturnsEnoent, testing::ext::TestSize.Level1)
{
    string path = testDir + "/nonexistent_subdir/file.txt";
    AtomicFile file(path);
    auto sw = file.StartWriteByPath();
    EXPECT_FALSE(sw.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_ReadFully_001
 * @tc.desc: Verify ReadFully reads full file content.
 */
HWTEST_F(AtomicFileTest, ReadFully_NonEmptyFile_ReturnsContent, testing::ext::TestSize.Level1)
{
    string path = testDir + "/ReadFully_001.txt";
    string content(8192 * 2, 'x');
    ASSERT_TRUE(FileUtils::CreateFile(path, content));

    AtomicFile file(path);
    string out;
    auto ret = file.ReadFully(out);
    EXPECT_TRUE(ret.IsSuccess());
    EXPECT_EQ(out, content);
}

/**
 * @tc.name: AtomicFileTest_ReadFully_002
 * @tc.desc: Verify ReadFully on empty file returns empty string.
 */
HWTEST_F(AtomicFileTest, ReadFully_EmptyFile_ReturnsEmpty, testing::ext::TestSize.Level1)
{
    string path = testDir + "/ReadFully_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    AtomicFile file(path);
    string out;
    auto ret = file.ReadFully(out);
    EXPECT_TRUE(ret.IsSuccess());
    EXPECT_TRUE(out.empty());
}

/**
 * @tc.name: AtomicFileTest_ReadFully_003
 * @tc.desc: Verify ReadFully on non-existent file returns ENOENT.
 */
HWTEST_F(AtomicFileTest, ReadFully_NonExistentFile_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/non_existent_ReadFully_003.txt";

    AtomicFile file(path);
    string out;
    auto ret = file.ReadFully(out);
    EXPECT_FALSE(ret.IsSuccess());
    EXPECT_TRUE(out.empty());
}

/**
 * @tc.name: AtomicFileTest_Delete_001
 * @tc.desc: Verify Delete removes base file.
 */
HWTEST_F(AtomicFileTest, Delete_RemovesBaseFile, testing::ext::TestSize.Level1)
{
    string path = testDir + "/Delete_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    AtomicFile file(path);
    auto ret = file.Delete();
    EXPECT_TRUE(ret.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(path));
}

/**
 * @tc.name: AtomicFileTest_Move_001
 * @tc.desc: Verify move semantics transfers ownership and original is empty.
 */
HWTEST_F(AtomicFileTest, Move_TransfersOwnership, testing::ext::TestSize.Level1)
{
    string path = testDir + "/Move_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "data"));

    AtomicFile fileA(path);
    AtomicFile fileB(std::move(fileA));
    EXPECT_EQ(fileB.GetPath(), path);
    string out;
    EXPECT_TRUE(fileB.ReadFully(out).IsSuccess());
    EXPECT_EQ(out, "data");
}

/**
 * @tc.name: AtomicFileTest_Move_002
 * @tc.desc: Verify move assignment operator transfers ownership.
 */
HWTEST_F(AtomicFileTest, Move_AssignmentTransfersOwnership, testing::ext::TestSize.Level1)
{
    string path = testDir + "/Move_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "assign"));

    AtomicFile fileA(path);
    AtomicFile fileB(testDir + "/placeholder.txt");
    fileB = std::move(fileA);
    EXPECT_EQ(fileB.GetPath(), path);
    string out;
    EXPECT_TRUE(fileB.ReadFully(out).IsSuccess());
    EXPECT_EQ(out, "assign");
}

/**
 * @tc.name: AtomicFileTest_FinishWrite_001
 * @tc.desc: Verify FinishWrite with null stream returns error.
 */
HWTEST_F(AtomicFileTest, FinishWrite_NullStream_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FinishWrite_Null.txt";
    AtomicFile file(path);
    auto ret = file.FinishWrite(nullptr);
    EXPECT_FALSE(ret.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_FailWrite_001
 * @tc.desc: Verify FailWrite with null stream returns error.
 */
HWTEST_F(AtomicFileTest, FailWrite_NullStream_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FailWrite_Null.txt";
    AtomicFile file(path);
    auto ret = file.FailWrite(nullptr);
    EXPECT_FALSE(ret.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_FinishWrite_002
 * @tc.desc: Verify FinishWrite on already committed stream returns error.
 */
HWTEST_F(AtomicFileTest, FinishWrite_AlreadyCommitted_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FinishWrite_Committed.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    auto fw1 = file.FinishWrite(stream);
    ASSERT_TRUE(fw1.IsSuccess());
    auto fw2 = file.FinishWrite(stream);
    EXPECT_FALSE(fw2.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_DiscardWrite_001
 * @tc.desc: Verify DiscardWrite on empty tmpPath returns success without error.
 */
HWTEST_F(AtomicFileTest, DiscardWrite_EmptyTmpPath_ReturnsSuccess, testing::ext::TestSize.Level1)
{
    string path = testDir + "/DiscardWrite_Empty.txt";
    AtomicFile file(path);
    file.Delete();
    auto ret = file.DiscardWrite();
    EXPECT_TRUE(ret.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_DiscardWrite_002
 * @tc.desc: Verify DiscardWrite on non-existent temp file returns success (ENOENT tolerated).
 */
HWTEST_F(AtomicFileTest, DiscardWrite_NonExistentTempFile_ReturnsSuccess, testing::ext::TestSize.Level1)
{
    string path = testDir + "/DiscardWrite_NonExistent.txt";
    AtomicFile file(path);
    auto ret = file.DiscardWrite();
    EXPECT_TRUE(ret.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_Delete_002
 * @tc.desc: Verify Delete when neither base nor temp file exists.
 */
HWTEST_F(AtomicFileTest, Delete_NonExistentFiles_ReturnsSuccess, testing::ext::TestSize.Level1)
{
    string path = testDir + "/Delete_NonExistent.txt";
    AtomicFile file(path);
    auto ret = file.Delete();
    EXPECT_TRUE(ret.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_ReadFully_004
 * @tc.desc: Verify ReadFully on file with content then check out is populated.
 */
HWTEST_F(AtomicFileTest, ReadFully_PartialRead_HandlesLargeFile, testing::ext::TestSize.Level1)
{
    string path = testDir + "/ReadFully_Large.txt";
    string content(8192 * 4, 'A');
    ASSERT_TRUE(FileUtils::CreateFile(path, content));

    AtomicFile file(path);
    string out;
    auto ret = file.ReadFully(out);
    EXPECT_TRUE(ret.IsSuccess());
    EXPECT_EQ(out.size(), content.size());
    EXPECT_EQ(out, content);
}

/**
 * @tc.name: AtomicFileTest_StartWriteFinishWrite_002
 * @tc.desc: Verify multiple StartWrite+FinishWrite cycles overwrite content correctly.
 */
HWTEST_F(AtomicFileTest, StartWriteFinishWrite_MultipleCycles_OverwriteCorrectly, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWriteFinishWrite_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "v1"));

    AtomicFile file(path);
    // First cycle: write "v2"
    auto sw1 = file.StartWrite();
    ASSERT_TRUE(sw1.IsSuccess());
    auto stream1 = sw1.GetData().value();
    stream1->write("v2", 2);
    ASSERT_TRUE(file.FinishWrite(stream1).IsSuccess());
    EXPECT_EQ(ReadFile(path), "v2");

    // Second cycle: write "v3"
    auto sw2 = file.StartWrite();
    ASSERT_TRUE(sw2.IsSuccess());
    auto stream2 = sw2.GetData().value();
    stream2->write("v3", 2);
    ASSERT_TRUE(file.FinishWrite(stream2).IsSuccess());
    EXPECT_EQ(ReadFile(path), "v3");
}

/**
 * @tc.name: AtomicFileTest_StartWriteFinishWrite_003
 * @tc.desc: Verify writing empty data overwrites existing file with empty content.
 */
HWTEST_F(AtomicFileTest, StartWriteFinishWrite_EmptyData_OverwritesToEmpty, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWriteFinishWrite_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    // Do not write any data
    auto fw = file.FinishWrite(stream);
    EXPECT_TRUE(fw.IsSuccess());
    EXPECT_TRUE(FileUtils::Exists(path));
    auto [succ, content] = FileUtils::ReadTextFileContent(path);
    EXPECT_TRUE(succ);
    EXPECT_TRUE(content.empty());
}

/**
 * @tc.name: AtomicFileTest_StartWriteFinishWrite_004
 * @tc.desc: Verify writing data larger than FdStreamBuf buffer (8192) succeeds.
 */
HWTEST_F(AtomicFileTest, StartWriteFinishWrite_LargeData_Succeeds, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWriteFinishWrite_004.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    string largeContent(100 * 1024, 'Z'); // 100KB, far exceeds 8192 buffer
    stream->write(largeContent.data(), largeContent.size());
    auto fw = file.FinishWrite(stream);
    EXPECT_TRUE(fw.IsSuccess());
    EXPECT_EQ(ReadFile(path), largeContent);
}

/**
 * @tc.name: AtomicFileTest_CommitWrite_001
 * @tc.desc: Verify CommitWrite returns error when temp file does not exist.
 */
HWTEST_F(AtomicFileTest, CommitWrite_NonExistentTempFile_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/CommitWrite_NonExistent.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto sw = file.StartWriteByPath();
    ASSERT_TRUE(sw.IsSuccess());
    string tmpPath = sw.GetData().value();
    // Manually remove the temp file before CommitWrite
    ASSERT_TRUE(FileUtils::RemoveAll(tmpPath));

    auto cw = file.CommitWrite();
    EXPECT_FALSE(cw.IsSuccess());
    // Original file should remain intact
    EXPECT_EQ(ReadFile(path), "keep");
}

/**
 * @tc.name: AtomicFileTest_StartWrite_002
 * @tc.desc: Verify StartWrite creates temp file and mkstemp replaces _XXXXXX suffix.
 */
HWTEST_F(AtomicFileTest, StartWrite_MkstempReplacesSuffix, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWrite_Mkstemp.txt";
    AtomicFile file(path);

    string tmpPathBefore = file.GetTmpPath();
    EXPECT_EQ(tmpPathBefore, path + "_XXXXXX");

    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();

    // After mkstemp, tmpPath should no longer contain the original _XXXXXX
    string tmpPathAfter = file.GetTmpPath();
    EXPECT_NE(tmpPathAfter, tmpPathBefore);
    EXPECT_TRUE(FileUtils::Exists(tmpPathAfter));

    // Clean up
    file.FailWrite(stream);
}

/**
 * @tc.name: AtomicFileTest_FailWrite_002
 * @tc.desc: Verify FailWrite resets tmpPath to basePath + suffix.
 */
HWTEST_F(AtomicFileTest, FailWrite_ResetsTmpPath, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FailWrite_ResetsTmpPath.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();

    // After StartWrite, tmpPath has been modified by mkstemp
    EXPECT_NE(file.GetTmpPath(), path + "_XXXXXX");

    auto fw = file.FailWrite(stream);
    EXPECT_TRUE(fw.IsSuccess());
    // After FailWrite, tmpPath should be reset
    EXPECT_EQ(file.GetTmpPath(), path + "_XXXXXX");
}

/**
 * @tc.name: AtomicFileTest_FinishWrite_003
 * @tc.desc: Verify FailWrite on already committed stream returns error.
 */
HWTEST_F(AtomicFileTest, FailWrite_AlreadyCommitted_ReturnsError, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FailWrite_Committed.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    auto fw1 = file.FinishWrite(stream);
    ASSERT_TRUE(fw1.IsSuccess());
    auto fw2 = file.FailWrite(stream);
    EXPECT_FALSE(fw2.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_Delete_003
 * @tc.desc: Verify Delete removes both temp and base files when both exist.
 */
HWTEST_F(AtomicFileTest, Delete_RemovesBothTempAndBaseFiles, testing::ext::TestSize.Level1)
{
    string path = testDir + "/Delete_BothFiles.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "base"));

    AtomicFile file(path);
    // Create a temp file via StartWriteByPath
    auto sw = file.StartWriteByPath();
    ASSERT_TRUE(sw.IsSuccess());
    string tmpPath = sw.GetData().value();
    EXPECT_TRUE(FileUtils::Exists(tmpPath));
    EXPECT_TRUE(FileUtils::Exists(path));

    auto ret = file.Delete();
    EXPECT_TRUE(ret.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(tmpPath));
    EXPECT_FALSE(FileUtils::Exists(path));
}

/**
 * @tc.name: AtomicFileTest_Move_003
 * @tc.desc: Verify moved-from object has cleared tmpPath.
 */
HWTEST_F(AtomicFileTest, Move_SourceTmpPathCleared, testing::ext::TestSize.Level1)
{
    string path = testDir + "/Move_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "data"));

    AtomicFile fileA(path);
    string tmpPathA = fileA.GetTmpPath();
    EXPECT_FALSE(tmpPathA.empty());

    AtomicFile fileB(std::move(fileA));
    // Moved-from object should have cleared tmpPath
    EXPECT_TRUE(fileA.GetTmpPath().empty());
    EXPECT_EQ(fileB.GetPath(), path);
    EXPECT_EQ(fileB.GetTmpPath(), tmpPathA);
}

/**
 * @tc.name: AtomicFileTest_Move_004
 * @tc.desc: Verify move self-assignment does not corrupt state.
 */
HWTEST_F(AtomicFileTest, Move_SelfAssignment_NoCorruption, testing::ext::TestSize.Level1)
{
    string path = testDir + "/Move_004.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "self"));

    AtomicFile file(path);
    AtomicFile &ref = file;
    file = std::move(ref); // self-move via reference to bypass -Wself-move
    EXPECT_EQ(file.GetPath(), path);
    string out;
    EXPECT_TRUE(file.ReadFully(out).IsSuccess());
    EXPECT_EQ(out, "self");
}

/**
 * @tc.name: AtomicFileTest_DiscardWrite_003
 * @tc.desc: Verify DiscardWrite after StartWriteByPath removes temp file.
 */
HWTEST_F(AtomicFileTest, DiscardWrite_AfterStartWriteByPath_RemovesTempFile, testing::ext::TestSize.Level1)
{
    string path = testDir + "/DiscardWrite_AfterByPath.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "keep"));

    AtomicFile file(path);
    auto sw = file.StartWriteByPath();
    ASSERT_TRUE(sw.IsSuccess());
    string tmpPath = sw.GetData().value();
    EXPECT_TRUE(FileUtils::Exists(tmpPath));

    auto dw = file.DiscardWrite();
    EXPECT_TRUE(dw.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(tmpPath));
    EXPECT_EQ(ReadFile(path), "keep");
}

/**
 * @tc.name: AtomicFileTest_ReadFully_005
 * @tc.desc: Verify ReadFully clears output string before reading.
 */
HWTEST_F(AtomicFileTest, ReadFully_ClearsOutputBeforeReading, testing::ext::TestSize.Level1)
{
    string path = testDir + "/ReadFully_Clears.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "new"));

    AtomicFile file(path);
    string out = "garbage_prefix";
    auto ret = file.ReadFully(out);
    EXPECT_TRUE(ret.IsSuccess());
    EXPECT_EQ(out, "new");
}

/**
 * @tc.name: AtomicFileTest_GetTmpPath_001
 * @tc.desc: Verify GetTmpPath returns basePath + suffix initially.
 */
HWTEST_F(AtomicFileTest, GetTmpPath_InitialValue_HasSuffix, testing::ext::TestSize.Level1)
{
    string path = testDir + "/GetTmpPath_001.txt";
    AtomicFile file(path);
    EXPECT_EQ(file.GetTmpPath(), path + "_XXXXXX");
}

/**
 * @tc.name: AtomicFileTest_StartWriteByPath_002
 * @tc.desc: Verify StartWriteByPath closes fd before returning path.
 */
HWTEST_F(AtomicFileTest, StartWriteByPath_ClosesFdBeforeReturn, testing::ext::TestSize.Level1)
{
    string path = testDir + "/StartWriteByPath_ClosesFd.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWriteByPath();
    ASSERT_TRUE(sw.IsSuccess());
    string tmpPath = sw.GetData().value();

    // The returned path should be a valid, accessible temp file (fd was closed, not leaked)
    EXPECT_TRUE(FileUtils::Exists(tmpPath));

    // We should be able to open and write to it independently
    {
        ofstream ofs(tmpPath, ios::binary);
        ASSERT_TRUE(ofs.is_open());
        ofs << "data";
        ofs.close();
    }
    EXPECT_EQ(ReadFile(tmpPath), "data");

    // Cleanup
    file.DiscardWrite();
}

/**
 * @tc.name: AtomicFileTest_StartWriteFinishWrite_BinaryData_PreservesNullBytes
 * @tc.desc: Verify writing binary data containing '\0' bytes survives the atomic write + rename cycle.
 */
HWTEST_F(AtomicFileTest, StartWriteFinishWrite_BinaryData_PreservesNullBytes, testing::ext::TestSize.Level1)
{
    string path = testDir + "/BinaryData.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    // Binary payload with embedded NUL bytes (text-mode read would truncate at first '\0')
    string binaryData;
    binaryData.push_back('H');
    binaryData.push_back('\0');
    binaryData.push_back('e');
    binaryData.push_back('\0');
    binaryData.push_back('l');
    binaryData.push_back('\0');
    binaryData.push_back('l');
    binaryData.push_back('\0');
    binaryData.push_back('o');
    // Add a trailing block of 100 bytes with mixed content
    for (int i = 0; i < 100; ++i) {
        binaryData.push_back(static_cast<char>(i % 256));
    }

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();
    stream->write(binaryData.data(), binaryData.size());
    auto fw = file.FinishWrite(stream);
    EXPECT_TRUE(fw.IsSuccess());

    // Read back in binary mode so NUL bytes are preserved
    string actual = ReadFileBinary(path);
    EXPECT_EQ(actual.size(), binaryData.size());
    EXPECT_EQ(actual, binaryData);
}

/**
 * @tc.name: AtomicFileTest_StartWriteFinishWrite_MultipleWrites_ConcatenatesCorrectly
 * @tc.desc: Verify multiple stream->write calls in a single session concatenate correctly.
 */
HWTEST_F(AtomicFileTest, StartWriteFinishWrite_MultipleWrites_ConcatenatesCorrectly, testing::ext::TestSize.Level1)
{
    string path = testDir + "/MultipleWrites.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    auto sw = file.StartWrite();
    ASSERT_TRUE(sw.IsSuccess());
    auto stream = sw.GetData().value();

    // Write in 3 separate calls — exercises FdStreamBuf buffer flushing across boundaries
    stream->write("Part1-", 6);
    stream->write("Part2-", 6);
    // Third chunk is larger than the FdStreamBuf buffer (8192) to force a real flush
    string largeChunk(9000, 'X');
    largeChunk += "-Part3";
    stream->write(largeChunk.data(), largeChunk.size());

    auto fw = file.FinishWrite(stream);
    EXPECT_TRUE(fw.IsSuccess());

    string expected = "Part1-Part2-" + string(9000, 'X') + "-Part3";
    string actual = ReadFileBinary(path);
    EXPECT_EQ(actual.size(), expected.size());
    EXPECT_EQ(actual, expected);
}

/**
 * @tc.name: AtomicFileTest_StartWriteFinishWrite_ThenReadFully_RoundTrip
 * @tc.desc: Verify data written via StartWrite/FinishWrite can be read back via ReadFully on the same object.
 */
HWTEST_F(AtomicFileTest, StartWriteFinishWrite_ThenReadFully_RoundTrip, testing::ext::TestSize.Level1)
{
    string path = testDir + "/WriteReadRoundTrip.txt";

    AtomicFile file(path);
    string content = "round-trip verification content";
    {
        auto sw = file.StartWrite();
        ASSERT_TRUE(sw.IsSuccess());
        auto stream = sw.GetData().value();
        stream->write(content.data(), content.size());
        auto fw = file.FinishWrite(stream);
        ASSERT_TRUE(fw.IsSuccess());
    }

    // Read back through the same AtomicFile object's ReadFully interface
    string out;
    auto rr = file.ReadFully(out);
    EXPECT_TRUE(rr.IsSuccess());
    EXPECT_EQ(out.size(), content.size());
    EXPECT_EQ(out, content);
}

/**
 * @tc.name: AtomicFileTest_Discard_ZeroFdTag_001
 * @tc.desc: Verify Discard closes an fd without an fdsan tag (covers zero-tag warning path).
 */
HWTEST_F(AtomicFileTest, Discard_ZeroFdTag_ClosesWithZeroTagWarning, testing::ext::TestSize.Level1)
{
    string tmpPath = testDir + "/Discard_ZeroFdTag_001_XXXXXX";
    ASSERT_TRUE(FileUtils::CreateFile(tmpPath, "data"));
    int fd = open(tmpPath.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    {
        AtomicWriteStream stream(fd, tmpPath);
        stream.Discard();
    }
    // Discard removed the temp file even though the fd carried no fdsan tag
    EXPECT_FALSE(FileUtils::Exists(tmpPath));
}

/**
 * @tc.name: AtomicFileTest_Discard_NegativeFd_001
 * @tc.desc: Verify Discard skips fd closing when fd is negative (covers fd_ < 0 guard).
 */
HWTEST_F(AtomicFileTest, Discard_NegativeFd_SkipsCloseAndRemovesTmpFile, testing::ext::TestSize.Level1)
{
    string tmpPath = testDir + "/Discard_NegativeFd_001_XXXXXX";
    ASSERT_TRUE(FileUtils::CreateFile(tmpPath, "data"));

    AtomicWriteStream stream(-1, tmpPath);
    stream.Discard();
    // No fd to close; temp file is still removed via tmpPath
    EXPECT_FALSE(FileUtils::Exists(tmpPath));
}

/**
 * @tc.name: AtomicFileTest_Discard_EmptyTmpPath_001
 * @tc.desc: Verify Discard skips remove when tmpPath is empty (covers empty tmpPath guard).
 */
HWTEST_F(AtomicFileTest, Discard_EmptyTmpPath_SkipsRemove, testing::ext::TestSize.Level1)
{
    string sentinelPath = testDir + "/Discard_EmptyTmpPath_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(sentinelPath, "sentinel"));
    int fd = open(sentinelPath.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);

    AtomicWriteStream stream(fd, "");
    stream.Discard();
    // Empty tmpPath: nothing should be removed; the sentinel file stays on disk
    EXPECT_TRUE(FileUtils::Exists(sentinelPath));
}

/**
 * @tc.name: AtomicFileTest_StartWrite_EmptyPath_001
 * @tc.desc: Verify StartWrite with empty path fails with ENOENT (13900002), matching XTS contract.
 */
HWTEST_F(AtomicFileTest, StartWrite_EmptyPath_ReturnsEnoent, testing::ext::TestSize.Level1)
{
    AtomicFile file("");
    auto sw = file.StartWrite();
    EXPECT_FALSE(sw.IsSuccess());
    EXPECT_EQ(sw.GetError().GetErrNo(), 13900002); // ENOENT
}

/**
 * @tc.name: AtomicFileTest_SyncParentDir_EmptyParent_001
 * @tc.desc: Verify SyncParentDir succeeds immediately when base path has no directory prefix.
 */
HWTEST_F(AtomicFileTest, SyncParentDir_EmptyParent_ReturnsSuccess, testing::ext::TestSize.Level1)
{
    // Relative name without any directory component: parent_path() is empty
    AtomicFile file("SyncParentDir_EmptyParent_001_relative.txt");
    auto ret = file.SyncParentDir();
    EXPECT_TRUE(ret.IsSuccess());
}

/**
 * @tc.name: AtomicFileTest_SyncParentDir_OpenFail_001
 * @tc.desc: Verify SyncParentDir returns ENOTDIR when parent path is a plain file.
 */
HWTEST_F(AtomicFileTest, SyncParentDir_ParentIsPlainFile_ReturnsNotdirError, testing::ext::TestSize.Level1)
{
    string plainFile = testDir + "/SyncParentDir_PlainFile_001";
    ASSERT_TRUE(FileUtils::CreateFile(plainFile, "not-a-dir"));

    // Parent "directory" of the target path is actually a plain file:
    // open(O_DIRECTORY) fails with ENOTDIR
    AtomicFile file(plainFile + "/child.txt");
    auto ret = file.SyncParentDir();
    EXPECT_FALSE(ret.IsSuccess());
    EXPECT_EQ(ret.GetError().GetErrNo(), 13900018); // ENOTDIR
}

/**
 * @tc.name: AtomicFileTest_FinishWrite_NegativeFd_001
 * @tc.desc: Verify FinishWrite with a negative-fd stream skips fsync/close and still renames.
 */
HWTEST_F(AtomicFileTest, FinishWrite_NegativeFd_SkipsFsyncCloseAndRenames, testing::ext::TestSize.Level1)
{
    string path = testDir + "/FinishWrite_NegativeFd_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "old"));

    AtomicFile file(path);
    // Prepare the temp file manually at the expected _XXXXXX tmpPath
    string tmpPath = file.GetTmpPath();
    ASSERT_TRUE(FileUtils::CreateFile(tmpPath, "new"));

    auto ws = std::make_shared<AtomicWriteStream>(-1, tmpPath);
    auto stream = std::static_pointer_cast<std::ostream>(ws);
    // No data written: flush on an empty buffer succeeds without touching the fd
    auto fw = file.FinishWrite(stream);
    EXPECT_TRUE(fw.IsSuccess());
    // DoRename still ran and replaced the base file content
    EXPECT_EQ(ReadFile(path), "new");
    EXPECT_FALSE(FileUtils::Exists(tmpPath));
}
}
