/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "atomic_file.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "atomic_write_stream.h"
#include "fdtag_func.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace FileApi {
namespace fs = std::filesystem;

using OHOS::FileManagement::ModuleFileIO::FdTagFunc;
using OHOS::FileManagement::ModuleFileIO::PREFIX_ADDR;

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM) && !defined(PREVIEW_LINUX)
static void SetFdTagSafe(int fd, uint64_t tag)
{
    FdTagFunc::SetFdTag(fd, tag);
    fdsan_exchange_owner_tag(fd, 0, tag);
}

static int CloseWithFdsan(int fd)
{
    if (fd < 0) {
        return -1;
    }
    uint64_t tag = FdTagFunc::GetFdTag(fd);
    FdTagFunc::SetFdTag(fd, 0);
    if (tag == 0) {
        HILOGW("Closing fd with zero tag, may cause fdsan warning, fd:%{public}d", fd);
    }
    return fdsan_close_with_tag(fd, tag);
}
#else
static void SetFdTagSafe(int fd, uint64_t tag) {}

static int CloseWithFdsan(int fd)
{
    if (fd < 0) {
        return -1;
    }
    return close(fd);
}
#endif

void AtomicWriteStream::Discard()
{
    committed_ = true;
    if (fd_ >= 0) {
        CloseWithFdsan(fd_);
        fd_ = -1;
    }
    if (!tmpPath_.empty()) {
        if (remove(tmpPath_.c_str()) != 0 && errno != ENOENT) {
            HILOGW("Failed to remove temp file, err:%{public}d", errno);
        }
    }
}

AtomicFile::AtomicFile(std::string path) : basePath_(std::move(path))
{
    ResetTmpPath();
}

AtomicFile::~AtomicFile() = default;

AtomicFile::AtomicFile(AtomicFile &&other) noexcept
    : basePath_(other.basePath_), tmpPath_(std::move(other.tmpPath_))
{
    other.tmpPath_.clear();
}

AtomicFile &AtomicFile::operator=(AtomicFile &&other) noexcept
{
    if (this != &other) {
        basePath_ = other.basePath_;
        tmpPath_ = std::move(other.tmpPath_);
        other.tmpPath_.clear();
    }
    return *this;
}

std::string AtomicFile::GetPath() const { return basePath_; }

std::string AtomicFile::GetTmpPath() const { return tmpPath_; }

void AtomicFile::ResetTmpPath() { tmpPath_ = basePath_ + TEMP_FILE_SUFFIX; }

FsResult<int> AtomicFile::CreateTempFile()
{
    fs::path filePath = tmpPath_;
    fs::path parentPath = filePath.parent_path();
    if (access(parentPath.c_str(), F_OK) != 0) {
        HILOGE("Parent directory does not exist, err:%{public}d", errno);
        return FsResult<int>::Error(ENOENT);
    }

    std::vector<char> tmpl(tmpPath_.begin(), tmpPath_.end());
    tmpl.push_back('\0');
    int fd = mkstemp(tmpl.data());
    if (fd < 0) {
        HILOGE("Failed to create temp file, err:%{public}d", errno);
        return FsResult<int>::Error(errno);
    }
    tmpPath_ = tmpl.data();

    auto syncRet = SyncParentDir();
    if (!syncRet.IsSuccess()) {
        close(fd);
        remove(tmpPath_.c_str());
        return FsResult<int>::Error(syncRet.GetError().GetErrNo());
    }
    return FsResult<int>::Success(fd);
}

FsResultVoid AtomicFile::SyncParentDir()
{
    fs::path parentPath = fs::path(basePath_).parent_path();
    if (parentPath.empty()) {
        return FsResultVoid::Success();
    }
    int dirFd = open(parentPath.c_str(), O_RDONLY | O_DIRECTORY);
    if (dirFd < 0) {
        HILOGE("Failed to open parent dir, err:%{public}d", errno);
        return FsResultVoid::Error(errno);
    }
    int ret = fsync(dirFd);
    int err = errno;
    close(dirFd);
    if (ret != 0) {
        HILOGE("Failed to fsync parent dir, err:%{public}d", err);
        return FsResultVoid::Error(err);
    }
    return FsResultVoid::Success();
}

FsResultVoid AtomicFile::DoRename()
{
    if (rename(tmpPath_.c_str(), basePath_.c_str()) != 0) {
        HILOGE("Failed to rename, err:%{public}d", errno);
        return FsResultVoid::Error(errno);
    }
    ResetTmpPath();
    return FsResultVoid::Success();
}

static uint64_t MakeFdTag(const AtomicFile *file)
{
    return PREFIX_ADDR | static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(file));
}

FsResult<std::shared_ptr<std::ostream>> AtomicFile::StartWrite()
{
    auto createResult = CreateTempFile();
    if (!createResult.IsSuccess()) {
        return FsResult<std::shared_ptr<std::ostream>>::Error(createResult.GetError().GetErrNo());
    }
    int fd = createResult.GetData().value();
    uint64_t tag = MakeFdTag(this);
    SetFdTagSafe(fd, tag);

    auto stream = std::make_shared<AtomicWriteStream>(fd, tmpPath_);
    return FsResult<std::shared_ptr<std::ostream>>::Success(
        std::static_pointer_cast<std::ostream>(stream));
}

FsResultVoid AtomicFile::FinishWrite(std::shared_ptr<std::ostream> stream)
{
    if (stream == nullptr) {
        HILOGE("Invalid stream");
        return FsResultVoid::Error(EINVAL);
    }
    auto *aws = static_cast<AtomicWriteStream *>(stream.get());
    if (aws == nullptr || aws->IsCommitted()) {
        HILOGE("Invalid or already committed stream");
        return FsResultVoid::Error(EINVAL);
    }

    if (!stream->flush()) {
        HILOGE("Failed to flush stream");
        aws->Discard();
        return FsResultVoid::Error(EIO);
    }
    int fd = aws->GetFd();
    if (fd >= 0 && fsync(fd) != 0) {
        HILOGE("Failed to fsync data, err:%{public}d", errno);
        aws->Discard();
        return FsResultVoid::Error(errno);
    }

    if (fd >= 0) {
        CloseWithFdsan(fd);
        aws->MarkCommitted();
    }

    auto renameResult = DoRename();
    if (!renameResult.IsSuccess()) {
        return renameResult;
    }
    return SyncParentDir();
}

FsResultVoid AtomicFile::FailWrite(std::shared_ptr<std::ostream> stream)
{
    if (stream == nullptr) {
        HILOGE("Invalid stream");
        return FsResultVoid::Error(EINVAL);
    }
    auto *aws = static_cast<AtomicWriteStream *>(stream.get());
    if (aws == nullptr || aws->IsCommitted()) {
        HILOGE("Invalid or already committed stream");
        return FsResultVoid::Error(EINVAL);
    }
    aws->Discard();
    ResetTmpPath();
    return FsResultVoid::Success();
}

FsResult<std::string> AtomicFile::StartWriteByPath()
{
    auto createResult = CreateTempFile();
    if (!createResult.IsSuccess()) {
        return FsResult<std::string>::Error(createResult.GetError().GetErrNo());
    }
    int fd = createResult.GetData().value();
    uint64_t tag = MakeFdTag(this);
    SetFdTagSafe(fd, tag);
    CloseWithFdsan(fd);
    return FsResult<std::string>::Success(tmpPath_);
}

FsResultVoid AtomicFile::CommitWrite()
{
    int fd = open(tmpPath_.c_str(), O_WRONLY);
    if (fd < 0) {
        HILOGE("Failed to open temp file, err:%{public}d", errno);
        return FsResultVoid::Error(errno);
    }
    uint64_t tag = MakeFdTag(this);
    SetFdTagSafe(fd, tag);

    int ret = fsync(fd);
    int err = errno;
    CloseWithFdsan(fd);
    if (ret != 0) {
        HILOGE("Failed to fsync data, err:%{public}d", err);
        remove(tmpPath_.c_str());
        return FsResultVoid::Error(err);
    }

    auto renameResult = DoRename();
    if (!renameResult.IsSuccess()) {
        return renameResult;
    }
    return SyncParentDir();
}

FsResultVoid AtomicFile::DiscardWrite()
{
    if (tmpPath_.empty()) {
        return FsResultVoid::Success();
    }
    if (remove(tmpPath_.c_str()) != 0 && errno != ENOENT) {
        HILOGE("Failed to remove temp file, err:%{public}d", errno);
        return FsResultVoid::Error(errno);
    }
    ResetTmpPath();
    return FsResultVoid::Success();
}

FsResultVoid AtomicFile::ReadFully(std::string &out)
{
    out.clear();
    int fd = open(basePath_.c_str(), O_RDONLY);
    if (fd < 0) {
        HILOGE("Failed to open file, err:%{public}d", errno);
        return FsResultVoid::Error(errno);
    }

    struct stat fileStat {};
    if (fstat(fd, &fileStat) != 0) {
        HILOGE("Failed to stat file, err:%{public}d", errno);
        close(fd);
        return FsResultVoid::Error(errno);
    }

    if (fileStat.st_size <= 0) {
        close(fd);
        return FsResultVoid::Success();
    }

    if (fileStat.st_size > static_cast<off_t>(SSIZE_MAX)) {
        HILOGE("File size too large, size:%{public}lld", static_cast<long long>(fileStat.st_size));
        close(fd);
        return FsResultVoid::Error(EFBIG);
    }

    out.resize(static_cast<size_t>(fileStat.st_size));
    size_t total = 0;
    while (total < out.size()) {
        ssize_t n = read(fd, &out[total], out.size() - total);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            HILOGE("Failed to read file, err:%{public}d", errno);
            close(fd);
            return FsResultVoid::Error(errno);
        }
        if (n == 0) {
            break;
        }
        total += static_cast<size_t>(n);
    }
    close(fd);
    out.resize(total);
    return FsResultVoid::Success();
}

FsResultVoid AtomicFile::Delete()
{
    bool errFlag = false;
    std::error_code ec;
    if (fs::exists(tmpPath_, ec) && !fs::remove(tmpPath_, ec)) {
        errFlag = true;
    }
    if (fs::exists(basePath_, ec) && !fs::remove(basePath_, ec)) {
        errFlag = true;
    }
    if (errFlag) {
        HILOGE("Failed to remove file, err:%{public}s", ec.message().c_str());
        return FsResultVoid::Error(ec.value());
    }
    tmpPath_.clear();
    basePath_.clear();
    return FsResultVoid::Success();
}

} // namespace FileApi
} // namespace FileManagement
} // namespace OHOS
