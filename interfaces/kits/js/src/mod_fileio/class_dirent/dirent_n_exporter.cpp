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

#include "dirent_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>

#include "common_func.h"
#include "dirent_entity.h"
#include "log.h"
#include "n_class.h"
#include "n_func_arg.h"
#include "securec.h"
#include "uni_error.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static DirentEntity *GetDirentEntity(napi_env env, napi_callback_info cbinfo)
{
    NFuncArg funcArg(env, cbinfo);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto direntEntity = NClass::GetEntityOf<DirentEntity>(env, funcArg.GetThisVar());
    if (!direntEntity) {
        UniError(EIO).ThrowErr(env, "Cannot get entity of Dirent");
        return nullptr;
    }
    return direntEntity;
}

static napi_value CheckDirentDType(napi_env env, napi_callback_info cbinfo, unsigned char dType)
{
    DirentEntity *direntEntity = GetDirentEntity(env, cbinfo);
    if (!direntEntity) {
        return nullptr;
    }

    return NVal::CreateBool(env, direntEntity->dirent_.d_type == dType).val_;
}

napi_value DirentNExporter::isBlockDevice(napi_env env, napi_callback_info cbinfo)
{
    return CheckDirentDType(env, cbinfo, DT_BLK);
}

napi_value DirentNExporter::isCharacterDevice(napi_env env, napi_callback_info cbinfo)
{
    return CheckDirentDType(env, cbinfo, DT_CHR);
}

napi_value DirentNExporter::isDirectory(napi_env env, napi_callback_info cbinfo)
{
    return CheckDirentDType(env, cbinfo, DT_DIR);
}

napi_value DirentNExporter::isFIFO(napi_env env, napi_callback_info cbinfo)
{
    return CheckDirentDType(env, cbinfo, DT_FIFO);
}

napi_value DirentNExporter::isFile(napi_env env, napi_callback_info cbinfo)
{
    return CheckDirentDType(env, cbinfo, DT_REG);
}

napi_value DirentNExporter::isSocket(napi_env env, napi_callback_info cbinfo)
{
    return CheckDirentDType(env, cbinfo, DT_SOCK);
}

napi_value DirentNExporter::isSymbolicLink(napi_env env, napi_callback_info cbinfo)
{
    return CheckDirentDType(env, cbinfo, DT_LNK);
}

napi_value DirentNExporter::GetName(napi_env env, napi_callback_info cbinfo)
{
    DirentEntity *direntEntity = GetDirentEntity(env, cbinfo);
    if (!direntEntity) {
        return nullptr;
    }
    return NVal::CreateUTF8String(env, direntEntity->dirent_.d_name).val_;
}

napi_value DirentNExporter::Constructor(napi_env env, napi_callback_info cbinfo)
{
    NFuncArg funcArg(env, cbinfo);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto direntEntity = make_unique<DirentEntity>();
    if (!NClass::SetEntityFor<DirentEntity>(env, funcArg.GetThisVar(), move(direntEntity))) {
        UniError(EIO).ThrowErr(env, "INNER BUG. Failed to wrap entity for obj dirent");
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool DirentNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("isBlockDevice", isBlockDevice),
        NVal::DeclareNapiFunction("isCharacterDevice", isCharacterDevice),
        NVal::DeclareNapiFunction("isDirectory", isDirectory),
        NVal::DeclareNapiFunction("isFIFO", isFIFO),
        NVal::DeclareNapiFunction("isFile", isFile),
        NVal::DeclareNapiFunction("isSocket", isSocket),
        NVal::DeclareNapiFunction("isSymbolicLink", isSymbolicLink),

        NVal::DeclareNapiGetter("name", GetName),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_,
                                                className,
                                                DirentNExporter::Constructor,
                                                std::move(props));
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class Dirent");
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class Dirent");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string DirentNExporter::GetClassName()
{
    return DirentNExporter::className_;
}

DirentNExporter::DirentNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

DirentNExporter::~DirentNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
