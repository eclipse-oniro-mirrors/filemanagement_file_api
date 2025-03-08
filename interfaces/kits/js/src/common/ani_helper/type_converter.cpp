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

#include "type_converter.h"

#include <securec.h>
#include <cstring>
#include <optional>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {

std::tuple<bool, std::string> TypeConverter::ToUTF8String(ani_env *env, const ani_string &path)
{
    ani_size sz {};
    std::string result;
    auto status = env->String_GetUTF8Size(path, &sz);
    if (status != ANI_OK) {
        return { false, EMPTY_STRING };
    }
    result.resize(sz + 1);
    status = env->String_GetUTF8SubString(path, 0, sz, result.data(), result.size(), &sz);
    if (status != ANI_OK) {
        return { false, EMPTY_STRING };
    }
    result.resize(sz);
    return { true, std::move(result) };
}

std::tuple<bool, std::optional<int32_t>> TypeConverter::ToOptionalInt32(ani_env *env, const ani_object &intOp)
{
    ani_boolean isUndefined;
    env->Reference_IsUndefined(intOp, &isUndefined);
    if (isUndefined) {
        return { true, std::nullopt };
    }

    ani_int result;
    if (ANI_OK != env->Object_CallMethodByName_Int(intOp, "intValue", nullptr, &result)) {
        return { false, {} };
    }

    return { true, std::make_optional(result) };
}

std::tuple<bool, std::optional<int64_t>> TypeConverter::ToOptionalInt64(ani_env *env, const ani_object &longOp)
{
    ani_boolean isUndefined;
    env->Reference_IsUndefined(longOp, &isUndefined);
    if (isUndefined) {
        return { true, std::nullopt };
    }

    ani_long result;
    if (ANI_OK != env->Object_CallMethodByName_Long(longOp, "longValue", nullptr, &result)) {
        return { false, {} };
    }

    return { true, std::make_optional(result) };
}

std::tuple<bool, ani_string> TypeConverter::ToAniString(ani_env *env, std::string str)
{
    ani_string result;
    if (ANI_OK != env->String_NewUTF8(str.c_str(), str.size(), &result)) {
        return { false, {} };
    }
    return { true, std::move(result) };
}

std::tuple<bool, ani_string> TypeConverter::ToAniString(ani_env *env, const char *str)
{
    size_t length = std::strlen(str);
    ani_string result;
    if (ANI_OK != env->String_NewUTF8(str, length, &result)) {
        return { false, {} };
    }
    return { true, std::move(result) };
}

std::tuple<bool, FileInfo> TypeConverter::ToFileInfo(ani_env *env, const ani_object &pathOrFd)
{
    ani_class stringClass;
    env->FindClass("Lstd/core/String;", &stringClass);

    ani_class IntClass;
    env->FindClass("Lstd/core/Int;", &IntClass);

    ani_boolean isPath = false;
    env->Object_InstanceOf(pathOrFd, stringClass, &isPath);
    if (isPath) {
        auto [succ, path] = TypeConverter::ToUTF8String(env, static_cast<ani_string>(pathOrFd));
        if (!succ) {
            HILOGE("Parse file path failed");
            return { false, FileInfo { false, {}, {} } };
        }
        size_t length = path.length() + 1;
        auto chars = std::make_unique<char[]>(length);
        auto ret = strncpy_s(chars.get(), length, path.c_str(), length - 1);
        if (ret != EOK) {
            HILOGE("Copy file path failed!");
            return { false, FileInfo { false, {}, {} } };
        }
        return { true, FileInfo { true, move(chars), {} } };
    }

    ani_boolean isFd = false;
    env->Object_InstanceOf(pathOrFd, IntClass, &isFd);
    if (isFd) {
        ani_int fd;
        if (ANI_OK != env->Object_CallMethodByName_Int(pathOrFd, "intValue", nullptr, &fd)) {
            HILOGE("Parse file path failed");
            return { false, FileInfo { false, {}, {} } };
        }
        auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
        if (fdg == nullptr) {
            HILOGE("Failed to request heap memory.");
            return { false, FileInfo { false, {}, {} } };
        }
        return { true, FileInfo { false, {}, move(fdg) } };
    }
    return { false, FileInfo { false, {}, {} } };
}

std::tuple<bool, ArrayBuffer> TypeConverter::ToArrayBuffer(ani_env *env, ani_arraybuffer &buffer)
{
    void *buf = nullptr;
    ani_size length = 0;

    if (ANI_OK != env->ArrayBuffer_GetInfo(buffer, &buf, &length)) {
        return { false, ArrayBuffer { nullptr, 0 } };
    }
    return { true, ArrayBuffer { std::move(buf), length } };
}

} // namespace OHOS::FileManagement::ModuleFileIO::ANI