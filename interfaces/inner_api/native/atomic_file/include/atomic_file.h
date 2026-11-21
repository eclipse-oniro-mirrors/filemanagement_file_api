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

#ifndef INTERFACES_INNER_API_NATIVE_ATOMIC_FILE_INCLUDE_ATOMIC_FILE_H
#define INTERFACES_INNER_API_NATIVE_ATOMIC_FILE_INCLUDE_ATOMIC_FILE_H

#include <memory>
#include <ostream>
#include <string>

#include "fs_result.h"

namespace OHOS {
namespace FileManagement {
namespace FileApi {

inline constexpr const char *TEMP_FILE_SUFFIX = "_XXXXXX";

using FsResultVoid = OHOS::FileManagement::ModuleFileIO::FsResult<void>;
template <typename T>
using FsResult = OHOS::FileManagement::ModuleFileIO::FsResult<T>;

class AtomicFile final {
public:
    explicit AtomicFile(std::string path);
    ~AtomicFile();

    AtomicFile(AtomicFile &&other) noexcept;
    AtomicFile &operator=(AtomicFile &&other) noexcept;
    AtomicFile(const AtomicFile &) = delete;
    AtomicFile &operator=(const AtomicFile &) = delete;

    std::string GetPath() const;
    std::string GetTmpPath() const;

    FsResult<std::shared_ptr<std::ostream>> StartWrite();
    FsResultVoid FinishWrite(std::shared_ptr<std::ostream> stream);
    FsResultVoid FailWrite(std::shared_ptr<std::ostream> stream);

    FsResult<std::string> StartWriteByPath();
    FsResultVoid CommitWrite();
    FsResultVoid DiscardWrite();

    FsResultVoid ReadFully(std::string &out);
    FsResultVoid Delete();

private:
    std::string basePath_;
    std::string tmpPath_;

    void ResetTmpPath();
    FsResult<int> CreateTempFile();
    FsResultVoid SyncParentDir();
    FsResultVoid DoRename();
};

} // namespace FileApi
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_INNER_API_NATIVE_ATOMIC_FILE_INCLUDE_ATOMIC_FILE_H
