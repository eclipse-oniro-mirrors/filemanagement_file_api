/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_FILE_FILE_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_FILE_FILE_ENTITY_H

#include <unistd.h>

#include "file_helper/fd_guard.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
struct FileEntity {
    std::unique_ptr<FDGuard> fd_ = { nullptr };
    std::string path_;
    std::string uri_;
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
#endif