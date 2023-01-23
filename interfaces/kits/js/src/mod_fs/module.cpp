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

#include "common_func.h"

#include <memory>
#include <vector>

#include "class_file/file_n_exporter.h"
#include "class_stat/stat_n_exporter.h"
#include "class_stream/stream_n_exporter.h"
#include "filemgmt_libhilog.h"
#include "properties/prop_n_exporter.h"

using namespace std;

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
static napi_value Export(napi_env env, napi_value exports)
{
    InitOpenMode(env, exports);
    std::vector<unique_ptr<NExporter>> products;
    products.emplace_back(make_unique<PropNExporter>(env, exports));
    products.emplace_back(make_unique<FileNExporter>(env, exports));
    products.emplace_back(make_unique<StatNExporter>(env, exports));
    products.emplace_back(make_unique<StreamNExporter>(env, exports));

    for (auto &&product : products) {
        if (!product->Export()) {
            HILOGE("INNER BUG. Failed to export class %{public}s for module fileio", product->GetClassName().c_str());
            return nullptr;
        } else {
            HILOGI("Class %{public}s for module fileio has been exported", product->GetClassName().c_str());
        }
    }
    return exports;
}

NAPI_MODULE(fs, Export)
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS