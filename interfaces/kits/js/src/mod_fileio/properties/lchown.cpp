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

#include "lchown.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "n_async_work_callback.h"
#include "n_async_work_promise.h"
#include "n_func_arg.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value Lchown::Sync(napi_env env, napi_callback_info info)
{
    return NVal::CreateUndefined(env).val_;
}

napi_value Lchown::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::THREE, NARG_CNT::FOUR);
    auto cbExec = [](napi_env env) -> UniError {
        return UniError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        return { NVal::CreateUndefined(env) };
    };

    const string procedureName = "FileIOLchown";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::FOURTH]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
