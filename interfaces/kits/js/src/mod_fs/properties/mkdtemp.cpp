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

#include "mkdtemp.h"

#include <uv.h>

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

napi_value Mkdtemp::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, tmp, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    string path = tmp.get();
    std::unique_ptr<uv_fs_t, decltype(uv_fs_req_cleanup)*> mkdtemp_req = { new uv_fs_t, uv_fs_req_cleanup };
    int ret = uv_fs_mkdtemp(nullptr, mkdtemp_req.get(), const_cast<char *>(path.c_str()), nullptr);
    if (ret < 0) {
        HILOGE("Failed to create a temporary directory with path: %{public}s", path.c_str());
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUTF8String(env, mkdtemp_req->path).val_;
}

napi_value Mkdtemp::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetFirstArg, tmp, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!resGetFirstArg) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = make_shared<string>();
    auto cbExec = [path = tmp.get(), arg]() -> NError {
        std::unique_ptr<uv_fs_t, decltype(uv_fs_req_cleanup)*> mkdtemp_req = { new uv_fs_t, uv_fs_req_cleanup };
        int ret = uv_fs_mkdtemp(nullptr, mkdtemp_req.get(), const_cast<char *>(path), nullptr);
        if (ret < 0) {
            HILOGE("Failed to create a temporary directory with path: %{public}s", path);
            return NError(errno);
        } else {
            *arg = mkdtemp_req->path;
            return NError(ERRNO_NOERR);
        }
    };

    auto cbComplete = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUTF8String(env, *arg);
        }
    };

    const string procedureName = "FileIOmkdtemp";
    size_t argc = funcArg.GetArgc();
    NVal thisVar(env, funcArg.GetThisVar());
    if (argc == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS