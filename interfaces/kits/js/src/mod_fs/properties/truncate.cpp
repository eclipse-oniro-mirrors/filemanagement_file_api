/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "truncate.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static tuple<bool, FileInfo> ParseJsFile(napi_env env, napi_value pathOrFdFromJsArg)
{
    auto [isPath, path, ignore] = NVal(env, pathOrFdFromJsArg).ToUTF8StringPath();
    if (isPath) {
        return { true, FileInfo { true, move(path), {} } };
    }
    auto [isFd, fd] = NVal(env, pathOrFdFromJsArg).ToInt32();
    if (!isFd || fd < 0) {
        HILOGE("Invalid fd");
        NError(EINVAL).ThrowErr(env);
        return { false, FileInfo { false, {}, {} } };
    }
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
    if (fdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return { false, FileInfo { false, {}, {} } };
    }
    return { true, FileInfo { false, {}, move(fdg) } };
};

static NError TruncateCore(napi_env env, FileInfo &fileInfo, int64_t truncateLen)
{
    if (fileInfo.isPath) {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> open_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!open_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_open(nullptr, open_req.get(), fileInfo.path.get(), O_RDWR,
                             S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
        if (ret < 0) {
            HILOGE("Failed to open by libuv ret %{public}d", ret);
            return NError(ret);
        }
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> ftruncate_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!ftruncate_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        ret = uv_fs_ftruncate(nullptr, ftruncate_req.get(), ret, truncateLen, nullptr);
        if (ret < 0) {
            HILOGE("Failed to truncate file by path ret %{public}d", ret);
            return NError(ret);
        }
    } else {
        std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> ftruncate_req = {
            new uv_fs_t, CommonFunc::fs_req_cleanup };
        if (!ftruncate_req) {
            HILOGE("Failed to request heap memory.");
            return NError(ENOMEM);
        }
        int ret = uv_fs_ftruncate(nullptr, ftruncate_req.get(), fileInfo.fdg->GetFD(), truncateLen, nullptr);
        if (ret < 0) {
            HILOGE("Failed to truncate file by fd for libuv error %{public}d", ret);
            return NError(ret);
        }
    }
    return NError(ERRNO_NOERR);
}

napi_value Truncate::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    int64_t truncateLen = 0;
    if (funcArg.GetArgc() == NARG_CNT::TWO) {
        tie(succ, truncateLen) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt64(truncateLen);
        if (!succ || truncateLen < 0) {
            HILOGE("Invalid truncate length");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }
    auto err = TruncateCore(env, fileInfo, truncateLen);
    if (err) {
        err.ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value Truncate::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, fileInfo] = ParseJsFile(env, funcArg[NARG_POS::FIRST]);
    if (!succ) {
        return nullptr;
    }
    int64_t truncateLen = 0;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        tie(succ, truncateLen) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt64(truncateLen);
        if (!succ || truncateLen < 0) {
            HILOGE("Invalid truncate length");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
    }
    auto cbExec = [fileInfo = make_shared<FileInfo>(move(fileInfo)), truncateLen, env = env]() -> NError {
        return TruncateCore(env, *fileInfo, truncateLen);
    };
    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        } else {
            return NVal::CreateUndefined(env);
        }
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
        !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_TRUNCATE_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_TRUNCATE_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO