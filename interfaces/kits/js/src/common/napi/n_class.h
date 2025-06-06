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

#ifndef INTERFACES_KITS_JS_SRC_COMMON_NAPI_N_CLASS_H
#define INTERFACES_KITS_JS_SRC_COMMON_NAPI_N_CLASS_H

#include <map>

#include "log.h"
#include "uni_header.h"

namespace OHOS {
namespace DistributedFS {
class NClass final {
public:
    NClass(const NClass &) = delete;
    NClass &operator = (const NClass &) = delete;
    static NClass &GetInstance();

    static std::tuple<bool, napi_value> DefineClass(napi_env env,
                                                    std::string className,
                                                    napi_callback constructor,
                                                    std::vector<napi_property_descriptor> &&properties);
    static bool SaveClass(napi_env env, std::string className, napi_value exClass);
    static napi_value InstantiateClass(napi_env env, const std::string& className, const std::vector<napi_value>& args);

    template <class T> static T *GetEntityOf(napi_env env, napi_value objStat)
    {
        if (!env || !objStat) {
            HILOGE("Empty input: env %d, obj %d", env == nullptr, objStat == nullptr);
            return nullptr;
        }
        T *t = nullptr;
        napi_status status = napi_unwrap(env, objStat, (void **)&t);
        if (status != napi_ok) {
            HILOGE("Cannot umwarp for pointer: %d", status);
            return nullptr;
        }
        return t;
    }

    template <class T> static bool SetEntityFor(napi_env env, napi_value obj, std::unique_ptr<T> entity)
    {
        napi_status status = napi_wrap(
            env,
            obj,
            entity.release(),
            [](napi_env env, void *data, void *hint) {
                std::unique_ptr<T>(static_cast<T *>(data));
            },
            nullptr,
            nullptr);
        return status == napi_ok;
    }

    template <class T> static T *RemoveEntityOfFinal(napi_env env, napi_value objStat)
    {
        if (!env || !objStat) {
            HILOGD("Empty input: env %d,obj %d", env == nullptr, objStat == nullptr);
            return nullptr;
        }
        T *t = nullptr;
        napi_status status = napi_remove_wrap(env, objStat, (void **)&t);
        if (status != napi_ok) {
            HILOGD("Cannot umwrap for pointer: %d", status);
            return nullptr;
        }
        return t;
    }

private:
    NClass() = default;
    ~NClass() = default;
    std::map<std::string, napi_ref> exClassMap;
    std::mutex exClassMapLock;
};
} // namespace DistributedFS
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_COMMON_NAPI_N_CLASS_H