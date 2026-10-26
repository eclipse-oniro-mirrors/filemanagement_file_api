/*
* Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <cstdio>
#include "oh_archive.h"
#include "oh_archive_errcode.h"
#include "unistd.h"

OH_Archive_ProgressType progressHandler(int progress, void *userData)
{
    return OH_ARCHIVE_PROGRESS_CONTINUE;
}

TEST(ARCHIVE_ZIP_WRITER_TEST, test_archive_writer_zip_compress_progress)
{
    const char *file = "./4gfile";
    const char* infile[] = {file};
    const char* outfile = "4gfile.zip";
    OH_Archive_ErrCode ret = OH_ARCHIVE_OK;
    FILE *fileHandle = fopen("./4gfile", "w");
    ASSERT_NE(fileHandle, nullptr);

    uint64_t size = 4.1 * 1024 * 1024;
    char buf[1024] = {0};
    uint64_t writeSize = 0;
    while (writeSize < size) {
        fwrite(buf, sizeof(char), 1024, fileHandle);
        writeSize += 1;
    }
    fclose(fileHandle);

    OH_Archive_Writer_Ctx ctx = OH_Archive_Writer_OpenFile(outfile, OH_ARCHIVE_OPEN_MODE_CREATE, OH_ARCHIVE_FMT_ZIP);
    ASSERT_NE(ctx, nullptr);
    ret = OH_Archive_Writer_SetProgressHandlerWithData(ctx, progressHandler, nullptr);
    // 设置压缩算法
    ret = OH_Archive_Writer_SetCompressMethod(ctx, OH_ARCHIVE_COMPRESS_DEFLATE, -1);
    EXPECT_EQ(ret, OH_ARCHIVE_OK);
    ret = OH_Archive_Writer_Add(ctx, infile, 1);
    EXPECT_EQ(ret, OH_ARCHIVE_OK);
    ret = OH_Archive_Writer_Close(ctx);
    EXPECT_EQ(ret, OH_ARCHIVE_OK);
}

TEST(ARCHIVE_ZIP_WRITER_TEST, test_archive_writer_zip_compress_kongdir)
{
    const char *file = "./kong";
    const char* infile[] = {file};
    const char* outfile = "kong.zip";
    OH_Archive_ErrCode ret = OH_ARCHIVE_OK;
    int result = mkdir(file, 0777);
    if (result == 0) {
        OH_Archive_Writer_Ctx ctx =
            OH_Archive_Writer_OpenFile(outfile, OH_ARCHIVE_OPEN_MODE_CREATE, OH_ARCHIVE_FMT_ZIP);
        ASSERT_NE(ctx, nullptr);
        ret = OH_Archive_Writer_SetProgressHandlerWithData(ctx, progressHandler, nullptr);
        // 设置压缩算法
        ret = OH_Archive_Writer_SetCompressMethod(ctx, OH_ARCHIVE_COMPRESS_DEFLATE, -1);
        EXPECT_EQ(ret, OH_ARCHIVE_OK);
        ret = OH_Archive_Writer_Add(ctx, infile, 1);
        EXPECT_EQ(ret, OH_ARCHIVE_OK);
        ret = OH_Archive_Writer_Close(ctx);
        EXPECT_EQ(ret, OH_ARCHIVE_OK);
    }
}

// ---- 以下用例针对 zip writer 内部 StreamTell/StreamSeek 返回值处理 ----

static void RemoveFileIfExist(const char *path)
{
    if (access(path, F_OK) == 0) {
        remove(path);
    }
}

static bool CreateFileWithSize(const char *path, size_t size)
{
    FILE *fp = fopen(path, "wb");
    if (fp == nullptr) {
        return false;
    }
    char buf[4096] = {0};
    size_t written = 0;
    while (written < size) {
        size_t n = size - written > sizeof(buf) ? sizeof(buf) : size - written;
        if (fwrite(buf, 1, n, fp) != n) {
            (void)fclose(fp);
            return false;
        }
        written += n;
    }
    (void)fclose(fp);
    return true;
}

/*
 * DEFLATE 压缩→解压回读：验证 StreamTell 增加负值检查后正常路径不受影响
 */
TEST(ARCHIVE_ZIP_WRITER_TEST, test_archive_writer_zip_stream_tell_roundtrip_deflate)
{
    const char *src = "./retcheck_src.txt";
    const char *zip = "./retcheck_normal.zip";
    const char *dst = "./retcheck_out";
    ASSERT_TRUE(CreateFileWithSize(src, 1024 * 100));

    OH_Archive_Writer_Ctx ctx = OH_Archive_Writer_OpenFile(zip, OH_ARCHIVE_OPEN_MODE_CREATE, OH_ARCHIVE_FMT_ZIP);
    ASSERT_NE(ctx, nullptr);
    ASSERT_EQ(OH_Archive_Writer_SetProgressHandlerWithData(ctx, progressHandler, nullptr), OH_ARCHIVE_OK);
    ASSERT_EQ(OH_Archive_Writer_SetCompressMethod(ctx, OH_ARCHIVE_COMPRESS_DEFLATE, -1), OH_ARCHIVE_OK);
    const char *infile[] = {src};
    ASSERT_EQ(OH_Archive_Writer_Add(ctx, infile, 1), OH_ARCHIVE_OK);
    ASSERT_EQ(OH_Archive_Writer_Close(ctx), OH_ARCHIVE_OK);

    OH_Archive_Reader_Ctx rctx = OH_Archive_Reader_OpenFile(zip);
    ASSERT_NE(rctx, nullptr);
    EXPECT_EQ(OH_Archive_Reader_ExtractAllFile(rctx, dst), OH_ARCHIVE_OK);
    OH_Archive_Reader_Close(rctx);

    RemoveFileIfExist(src);
    RemoveFileIfExist(zip);
}

/*
 * STORE 方式压缩→解压回读
 */
TEST(ARCHIVE_ZIP_WRITER_TEST, test_archive_writer_zip_stream_tell_roundtrip_store)
{
    const char *src = "./retcheck_store_src.bin";
    const char *zip = "./retcheck_store.zip";
    const char *dst = "./retcheck_store_out";
    ASSERT_TRUE(CreateFileWithSize(src, 64 * 1024));

    OH_Archive_Writer_Ctx ctx = OH_Archive_Writer_OpenFile(zip, OH_ARCHIVE_OPEN_MODE_CREATE, OH_ARCHIVE_FMT_ZIP);
    ASSERT_NE(ctx, nullptr);
    ASSERT_EQ(OH_Archive_Writer_SetProgressHandlerWithData(ctx, progressHandler, nullptr), OH_ARCHIVE_OK);
    ASSERT_EQ(OH_Archive_Writer_SetCompressMethod(ctx, OH_ARCHIVE_NO_COMPRESSION, -1), OH_ARCHIVE_OK);
    const char *infile[] = {src};
    ASSERT_EQ(OH_Archive_Writer_Add(ctx, infile, 1), OH_ARCHIVE_OK);
    ASSERT_EQ(OH_Archive_Writer_Close(ctx), OH_ARCHIVE_OK);

    OH_Archive_Reader_Ctx rctx = OH_Archive_Reader_OpenFile(zip);
    ASSERT_NE(rctx, nullptr);
    EXPECT_EQ(OH_Archive_Reader_ExtractAllFile(rctx, dst), OH_ARCHIVE_OK);
    OH_Archive_Reader_Close(rctx);

    RemoveFileIfExist(src);
    RemoveFileIfExist(zip);
}

/*
 * 输出到不可写目录：写入失败必须报错，不得静默成功（StreamWrite 错误传播）
 */
TEST(ARCHIVE_ZIP_WRITER_TEST, test_archive_writer_zip_write_fail_expect_error)
{
    const char *src = "./retcheck_ro_src.txt";
    ASSERT_TRUE(CreateFileWithSize(src, 4096));
    const char *outfile = "/proc/retcheck_ro.zip"; // /proc 下无法创建文件

    OH_Archive_Writer_Ctx ctx = OH_Archive_Writer_OpenFile(outfile, OH_ARCHIVE_OPEN_MODE_CREATE, OH_ARCHIVE_FMT_ZIP);
    if (ctx == nullptr) {
        // OpenFile 内部创建输出流失败，同样属于错误正确传播
        SUCCEED();
        RemoveFileIfExist(src);
        return;
    }
    ASSERT_EQ(OH_Archive_Writer_SetCompressMethod(ctx, OH_ARCHIVE_COMPRESS_DEFLATE, -1), OH_ARCHIVE_OK);
    const char *infile[] = {src};
    EXPECT_NE(OH_Archive_Writer_Add(ctx, infile, 1), OH_ARCHIVE_OK);
    OH_Archive_Writer_Close(ctx);
    RemoveFileIfExist(src);
}

/*
 * 多文件（含空文件与目录）压缩→解压：多次 Add 下 StreamTell 偏移逐次递增正确
 */
TEST(ARCHIVE_ZIP_WRITER_TEST, test_archive_writer_zip_multi_entry_tell_offsets)
{
    const char *dir = "./retcheck_dir";
    ASSERT_EQ(mkdir(dir, 0777), 0);
    const char *empty = "./retcheck_empty.txt";
    ASSERT_TRUE(CreateFileWithSize(empty, 0));
    const char *normal = "./retcheck_normal2.txt";
    ASSERT_TRUE(CreateFileWithSize(normal, 8192));

    const char *zip = "./retcheck_multi.zip";
    const char *dst = "./retcheck_multi_out";

    OH_Archive_Writer_Ctx ctx = OH_Archive_Writer_OpenFile(zip, OH_ARCHIVE_OPEN_MODE_CREATE, OH_ARCHIVE_FMT_ZIP);
    ASSERT_NE(ctx, nullptr);
    ASSERT_EQ(OH_Archive_Writer_SetProgressHandlerWithData(ctx, progressHandler, nullptr), OH_ARCHIVE_OK);
    ASSERT_EQ(OH_Archive_Writer_SetCompressMethod(ctx, OH_ARCHIVE_COMPRESS_DEFLATE, -1), OH_ARCHIVE_OK);
    const char *infile[] = {dir, empty, normal};
    ASSERT_EQ(OH_Archive_Writer_Add(ctx, infile, 3), OH_ARCHIVE_OK);
    ASSERT_EQ(OH_Archive_Writer_Close(ctx), OH_ARCHIVE_OK);

    OH_Archive_Reader_Ctx rctx = OH_Archive_Reader_OpenFile(zip);
    ASSERT_NE(rctx, nullptr);
    EXPECT_EQ(OH_Archive_Reader_ExtractAllFile(rctx, dst), OH_ARCHIVE_OK);
    OH_Archive_Reader_Close(rctx);

    RemoveFileIfExist(empty);
    RemoveFileIfExist(normal);
    RemoveFileIfExist(zip);
}

/*
 * 3MB 大文件压缩：ZipEntryUpdateLocalHeader 中 Seek 往返后，
 * EOCD 中中央目录偏移/大小必须落在文件范围内（StreamTell 未失败的直接证据）
 */
TEST(ARCHIVE_ZIP_WRITER_TEST, test_archive_writer_zip_seek_tell_roundtrip_large)
{
    const char *src = "./retcheck_large_src.bin";
    const char *zip = "./retcheck_large.zip";
    ASSERT_TRUE(CreateFileWithSize(src, 3 * 1024 * 1024));

    OH_Archive_Writer_Ctx ctx = OH_Archive_Writer_OpenFile(zip, OH_ARCHIVE_OPEN_MODE_CREATE, OH_ARCHIVE_FMT_ZIP);
    ASSERT_NE(ctx, nullptr);
    ASSERT_EQ(OH_Archive_Writer_SetProgressHandlerWithData(ctx, progressHandler, nullptr), OH_ARCHIVE_OK);
    ASSERT_EQ(OH_Archive_Writer_SetCompressMethod(ctx, OH_ARCHIVE_COMPRESS_DEFLATE, 6), OH_ARCHIVE_OK);
    const char *infile[] = {src};
    ASSERT_EQ(OH_Archive_Writer_Add(ctx, infile, 1), OH_ARCHIVE_OK);
    ASSERT_EQ(OH_Archive_Writer_Close(ctx), OH_ARCHIVE_OK);

    // 校验 EOCD：偏移+大小不得超过文件实际长度
    FILE *fp = fopen(zip, "rb");
    ASSERT_NE(fp, nullptr);
    fseek(fp, -22, SEEK_END);
    unsigned char eocd[22];
    ASSERT_EQ(fread(eocd, 1, 22, fp), 22u);
    struct stat st;
    ASSERT_EQ(stat(zip, &st), 0);
    fclose(fp);
    ASSERT_EQ(eocd[0], 0x50); // 'P'
    ASSERT_EQ(eocd[1], 0x4B); // 'K'
    uint32_t cdSize = eocd[12] | (eocd[13] << 8) | (eocd[14] << 16) | ((uint32_t)eocd[15] << 24);
    uint32_t cdOffset = eocd[16] | (eocd[17] << 8) | (eocd[18] << 16) | ((uint32_t)eocd[19] << 24);
    EXPECT_GT(cdOffset + cdSize, 0u);
    EXPECT_LT(cdOffset + cdSize, (uint32_t)st.st_size + 1);

    RemoveFileIfExist(src);
    RemoveFileIfExist(zip);
}
