/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "fs_atomicfile.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <memory>
#include <securec.h>
#include <sys/stat.h>

#include "fs_atomicfile_entity.h"
#include "filemgmt_libfs.h"
#include "file_instantiator.h"
#include "file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace fs = std::filesystem;

const std::string READ_STREAM_CLASS = "ReadStream";
const std::string WRITE_STREAM_CLASS = "WriteStream";
const std::string TEMP_FILE_SUFFIX = "_XXXXXX";

void FsAtomicFile::SyncEntity()
{
    if (atomicFile_) {
        entity_.baseFileName = atomicFile_->GetPath();
        entity_.newFileName = atomicFile_->GetTmpPath();
    }
}

FsAtomicFileEntity *FsAtomicFile::GetEntity()
{
    SyncEntity();
    if (!atomicFile_) {
        return nullptr;
    }
    return &entity_;
}

void FsAtomicFile::FinalizeCallback(void *finalizeData, [[maybe_unused]] void *finalizeHint)
{
    BufferData *bufferData = static_cast<BufferData *>(finalizeData);
    delete bufferData;
}

string FsAtomicFile::GetPath()
{
    if (!atomicFile_) {
        return "";
    }
    return atomicFile_->GetPath();
}

FsResult<FsFile *> FsAtomicFile::GetBaseFile()
{
    if (atomicFile_ == nullptr) {
        HILOGE("Failed to get atomicFileEntity");
        return FsResult<FsFile *>::Error(UNKNOWN_ERR);
    }

    string basePath = atomicFile_->GetPath();
    if (basePath.size() >= PATH_MAX) {
        HILOGE("Base file name is too long");
        return FsResult<FsFile *>::Error(UNKNOWN_ERR);
    }

    auto absolutePath = std::make_unique<char[]>(PATH_MAX);
    char *result = realpath(basePath.c_str(), absolutePath.get());
    if (result == nullptr) {
        HILOGE("Failed to resolve real path, err:%{public}d", errno);
        return FsResult<FsFile *>::Error(errno);
    }

    int fd = open(result, O_RDONLY);
    if (fd < 0) {
        HILOGE("Failed to open file, err:%{public}d", errno);
        return FsResult<FsFile *>::Error(errno);
    }

    return FileInstantiator::InstantiateFile(fd, basePath, false);
}

FsResult<unique_ptr<BufferData>> FsAtomicFile::ReadFully()
{
    if (atomicFile_ == nullptr) {
        HILOGE("Failed to get atomicFile");
        return FsResult<unique_ptr<BufferData>>::Error(UNKNOWN_ERR);
    }

    string content;
    auto ret = atomicFile_->ReadFully(content);
    if (!ret.IsSuccess()) {
        return FsResult<unique_ptr<BufferData>>::Error(ret.GetError().GetErrNo());
    }

    auto bufferData = std::make_unique<BufferData>();
    if (!content.empty()) {
        bufferData->buffer = new (std::nothrow) uint8_t[content.size()];
        if (bufferData->buffer == nullptr) {
            HILOGE("Failed to allocate memory");
            return FsResult<unique_ptr<BufferData>>::Error(ENOMEM);
        }
        bufferData->length = content.size();
        if (memcpy_s(bufferData->buffer, content.size(), content.data(), content.size()) != EOK) {
            HILOGE("Failed to copy data");
            delete[] bufferData->buffer;
            bufferData->buffer = nullptr;
            bufferData->length = 0;
            return FsResult<unique_ptr<BufferData>>::Error(EIO);
        }
    }
    return FsResult<unique_ptr<BufferData>>::Success(move(bufferData));
}

FsResult<string> FsAtomicFile::StartWrite()
{
    if (atomicFile_ == nullptr) {
        HILOGE("Failed to get atomicFile");
        return FsResult<string>::Error(UNKNOWN_ERR);
    }
    auto ret = atomicFile_->StartWriteByPath();
    if (!ret.IsSuccess()) {
        return FsResult<string>::Error(ret.GetError().GetErrNo());
    }
    return FsResult<string>::Success(ret.GetData().value());
}

FsResult<void> FsAtomicFile::FinishWrite()
{
    if (atomicFile_ == nullptr) {
        HILOGE("Failed to get atomicFile");
        return FsResult<void>::Error(UNKNOWN_ERR);
    }
    auto ret = atomicFile_->CommitWrite();
    SyncEntity();
    if (!ret.IsSuccess()) {
        return FsResult<void>::Error(ret.GetError().GetErrNo());
    }
    return FsResult<void>::Success();
}

FsResult<void> FsAtomicFile::FailWrite()
{
    if (atomicFile_ == nullptr) {
        HILOGE("Failed to get atomicFile");
        return FsResult<void>::Error(UNKNOWN_ERR);
    }
    auto ret = atomicFile_->DiscardWrite();
    SyncEntity();
    if (!ret.IsSuccess()) {
        return FsResult<void>::Error(ret.GetError().GetErrNo());
    }
    return FsResult<void>::Success();
}

FsResult<void> FsAtomicFile::Delete()
{
    if (atomicFile_ == nullptr) {
        HILOGE("Failed to get atomicFile");
        return FsResult<void>::Error(UNKNOWN_ERR);
    }
    auto ret = atomicFile_->Delete();
    SyncEntity();
    if (!ret.IsSuccess()) {
        return FsResult<void>::Error(ret.GetError().GetErrNo());
    }
    return FsResult<void>::Success();
}

FsAtomicFile::FsAtomicFile(string path)
{
    atomicFile_ = std::make_unique<FileApi::AtomicFile>(std::move(path));
    SyncEntity();
}

FsResult<FsAtomicFile *> FsAtomicFile::Constructor(string path)
{
    auto file = new (std::nothrow) FsAtomicFile(move(path));
    if (file == nullptr) {
        HILOGE("Failed to create FsAtomicFile");
        return FsResult<FsAtomicFile *>::Error(ENOMEM);
    }
    return FsResult<FsAtomicFile *>::Success(file);
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
