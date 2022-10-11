/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_FILE_FILE_N_EXPORTER_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_FILE_FILE_N_EXPORTER_H

#include "../../common/napi/n_exporter.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
class FileNExporter final : public NExporter {
public:
    inline static const std::string className_ = "File";

    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info cbinfo);

    static napi_value GetFD(napi_env env, napi_callback_info cbinfo);
    static napi_value GetPath(napi_env env, napi_callback_info cbinfo);
    static napi_value GetUri(napi_env env, napi_callback_info cbinfo);

    FileNExporter(napi_env env, napi_value exports);
    ~FileNExporter() override;
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
#endif