/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_PROPERTIES_PROP_N_EXPORTER_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_PROPERTIES_PROP_N_EXPORTER_H

#include "n_async/n_ref.h"
#include "n_exporter.h"
#include "n_val.h"
#include "uni_error.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
struct AsyncIOWrtieArg {
    NRef refWriteArrayBuf_;
    std::unique_ptr<char[]> guardWriteStr_ = nullptr;
    ssize_t actLen = 0;

    explicit AsyncIOWrtieArg(NVal refWriteArrayBuf) : refWriteArrayBuf_(refWriteArrayBuf) {}
    explicit AsyncIOWrtieArg(std::unique_ptr<char[]> &&guardWriteStr) : guardWriteStr_(move(guardWriteStr)) {}
    ~AsyncIOWrtieArg() = default;
};

class PropNExporter final : public NExporter {
public:
    inline static const std::string className_ = "__properities__";

    static napi_value AccessSync(napi_env env, napi_callback_info info);
    static napi_value FchmodSync(napi_env env, napi_callback_info info);
    static napi_value FchownSync(napi_env env, napi_callback_info info);
    static napi_value MkdirSync(napi_env env, napi_callback_info info);
    static napi_value ReadSync(napi_env env, napi_callback_info info);
    static napi_value RenameSync(napi_env env, napi_callback_info info);
    static napi_value RmdirSync(napi_env env, napi_callback_info info);
    static napi_value UnlinkSync(napi_env env, napi_callback_info info);
    static napi_value FsyncSync(napi_env env, napi_callback_info info);
    static napi_value WriteSync(napi_env env, napi_callback_info info);
    static napi_value Access(napi_env env, napi_callback_info info);
    static napi_value Unlink(napi_env env, napi_callback_info info);
    static napi_value Mkdir(napi_env env, napi_callback_info info);
    static napi_value Read(napi_env env, napi_callback_info info);
    static napi_value Write(napi_env env, napi_callback_info info);
    bool ExportSync();
    bool ExportAsync();
    bool Export() override;
    std::string GetClassName() override;

    PropNExporter(napi_env env, napi_value exports);
    ~PropNExporter() = default;
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
#endif