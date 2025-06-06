/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_RANDOMACCESSFILE_RANDOMACCESSFILE_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_RANDOMACCESSFILE_RANDOMACCESSFILE_ENTITY_H

#include <unistd.h>

#include "fd_guard.h"
#include "n_val.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
const int64_t INVALID_POS = -1;
struct RandomAccessFileEntity {
    std::unique_ptr<DistributedFS::FDGuard> fd = {nullptr};
    int64_t filePointer = 0;
    int64_t start = INVALID_POS;
    int64_t end = INVALID_POS;
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif