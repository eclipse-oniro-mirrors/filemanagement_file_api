/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LISTFILE_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LISTFILE_CORE_H

#include <dirent.h>

#include "file_filter.h"
#include "filemgmt_libfs.h"
#include "fs_file_filter.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

struct NameListArg {
    struct dirent **namelist = { nullptr };
    int direntNum = 0;
};

constexpr int DEFAULT_SIZE = -1;
constexpr int DEFAULT_MODIFY_AFTER = -1;
struct OptionArgs {
    FileFilter filter =
        FileFilterBuilder().SetFileSizeOver(DEFAULT_SIZE).SetLastModifiedAfter(DEFAULT_MODIFY_AFTER).Build();
    int listNum = 0;
    int countNum = 0;
    bool recursion = false;
    std::string path = "";
    void Clear()
    {
        filter.FilterClear();
        filter.SetFileSizeOver(DEFAULT_SIZE);
        filter.SetLastModifiedAfter(DEFAULT_MODIFY_AFTER);
        listNum = 0;
        countNum = 0;
        recursion = false;
        path = "";
    }
};

struct FsListFileOptions {
    bool recursion = false;
    int64_t listNum = 0;
    optional<FsFileFilter> filter = nullopt;
};

class ListFileCore {
public:
    static FsResult<std::vector<std::string>> DoListFile(
        const std::string &path, const optional<FsListFileOptions> &opt = nullopt);
};

constexpr int FILTER_MATCH = 1;
constexpr int FILTER_DISMATCH = 0;
const int32_t MAX_SUFFIX_LENGTH = 256;

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_LISTFILE_CORE_H