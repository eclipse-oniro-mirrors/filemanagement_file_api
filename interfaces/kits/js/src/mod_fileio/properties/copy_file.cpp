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

#include "copy_file.h"

#include <cstring>
#include <fcntl.h>
#include <tuple>
#include <unistd.h>

#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "file_helper/fd_guard.h"
#include "n_async_work_callback.h"
#include "n_async_work_promise.h"
#include "napi/n_func_arg.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

namespace {
constexpr int COPY_BLOCK_SIZE = 4096;
}

struct FileInfo {
    bool isPath = false;
    unique_ptr<char[]> path = nullptr;
    FDGuard fdg;
};

static UniError CopyFileCore(FileInfo &srcFile, FileInfo &destFile)
{
    if (srcFile.isPath) {
        int ret = open(srcFile.path.get(), O_RDONLY);
        if (ret < 0) {
            return UniError(errno);
        }
        srcFile.fdg.SetFD(ret, true);
    }

    struct stat statbf;
    if (fstat(srcFile.fdg.GetFD(), &statbf) == -1) {
        return UniError(errno);
    }

    if (destFile.isPath) {
        int ret = open(destFile.path.get(), O_WRONLY | O_CREAT, statbf.st_mode);
        if (ret < 0) {
            return UniError(errno);
        }
        destFile.fdg.SetFD(ret, true);
    }

    auto copyBuf = make_unique<char[]>(COPY_BLOCK_SIZE);
    do {
        ssize_t readSize = read(srcFile.fdg.GetFD(), copyBuf.get(), COPY_BLOCK_SIZE);
        if (readSize == -1) {
            return UniError(errno);
        } else if (readSize == 0) {
            break;
        }
        ssize_t writeSize = write(destFile.fdg.GetFD(), copyBuf.get(), readSize);
        if (writeSize != readSize) {
            return UniError(errno);
        }
        if (readSize != COPY_BLOCK_SIZE) {
            break;
        }
    } while (true);

    return UniError(ERRNO_NOERR);
}

static tuple<bool, int32_t> ParseJsModeAndProm(napi_env env, const NFuncArg &funcArg)
{
    bool succ = false;
    int32_t mode = 0;
    if (funcArg.GetArgc() >= NARG_CNT::THREE) {
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(mode);
        if (!succ || mode) {
            return { false, mode };
        }
    }
    return { true, mode };
}

static tuple<bool, FileInfo> ParseJsOperand(napi_env env, NVal pathOrFdFromJsArg)
{
    auto [isPath, path, ignore] = pathOrFdFromJsArg.ToUTF8StringPath();
    if (isPath) {
        return {true, FileInfo{true, move(path), {}}};
    }

    auto [isFd, fd] = pathOrFdFromJsArg.ToInt32();
    if (isFd && fd > 0) {
        return {true, FileInfo{false, {}, {fd, false}}};
    }

    return {false, FileInfo{false, {}, {}}};
};

napi_value CopyFile::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succSrc, src] = ParseJsOperand(env, {env, funcArg[NARG_POS::FIRST]});
    auto [succDest, dest] = ParseJsOperand(env, {env, funcArg[NARG_POS::SECOND]});
    if (!succSrc || !succDest) {
        UniError(EINVAL).ThrowErr(env, "The first/second argument requires filepath/fd");
        return nullptr;
    }

    auto [succMode, mode] = ParseJsModeAndProm(env, funcArg);
    if (!succMode) {
        UniError(EINVAL).ThrowErr(env, "Invalid mode");
        return nullptr;
    }

    auto err = CopyFileCore(src, dest);
    if (err) {
        if (err.GetErrno(ERR_CODE_SYSTEM_POSIX) == ENAMETOOLONG) {
            UniError(EINVAL).ThrowErr(env, "Filename too long");
            return nullptr;
        }
        err.ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

class Para {
public:
    FileInfo src_;
    FileInfo dest_;

    Para(FileInfo src, FileInfo dest) : src_(move(src)), dest_(move(dest)) {};
};

napi_value CopyFile::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::FOUR)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succSrc, src] = ParseJsOperand(env, {env, funcArg[NARG_POS::FIRST]});
    auto [succDest, dest] = ParseJsOperand(env, {env, funcArg[NARG_POS::SECOND]});
    if (!succSrc || !succDest) {
        UniError(EINVAL).ThrowErr(env, "The first/second argument requires filepath/fd");
        return nullptr;
    }

    auto [succMode, mode] = ParseJsModeAndProm(env, funcArg);
    if (!succMode) {
        UniError(EINVAL).ThrowErr(env, "Invalid mode");
        return nullptr;
    }

    auto cbExec = [para = make_shared<Para>(move(src), move(dest))](napi_env env) -> UniError {
        return CopyFileCore(para->src_, para->dest_);
    };

    auto cbCompl = [](napi_env env, UniError err) -> NVal {
        if (err) {
            if (err.GetErrno(ERR_CODE_SYSTEM_POSIX) == ENAMETOOLONG) {
                return {env, err.GetNapiErr(env, "Filename too long")};
            }
            return {env, err.GetNapiErr(env)};
        }
        return {NVal::CreateUndefined(env)};
    };

    const string procedureName = "FileIOCopyFile";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::TWO || (funcArg.GetArgc() == NARG_CNT::THREE &&
        !NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::THREE) ? NARG_POS::THIRD : NARG_POS::FOURTH)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
