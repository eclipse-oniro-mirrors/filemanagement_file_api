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

#ifndef FILEMGMT_LIBN_N_CLASS_H
#define FILEMGMT_LIBN_N_CLASS_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

#include "n_napi.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
class NClass final {
public:
    NClass(const NClass&) = delete;
    NClass &operator=(const NClass&) = delete;
    static std::tuple<bool, napi_value> DefineClass(napi_env env,
                                                    std::string className,
                                                    napi_callback constructor,
                                                    std::vector<napi_property_descriptor> &&properties);
    static bool SaveClass(napi_env env, std::string className, napi_value exClass);
    static void CleanClass(void *arg);
    static napi_value InstantiateClass(napi_env env, const std::string& className, const std::vector<napi_value>& args);

    template <class T> static T *GetEntityOf(napi_env env, napi_value objStat)
    {
        std::lock_guard<std::mutex> lock(wrapLock);
        if (!env || !objStat || wrapRelease) {
            HILOGE("Empty input: env %d %d, obj %d", env == nullptr, wrapRelease, objStat == nullptr);
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
        std::lock_guard<std::mutex> lock(wrapLock);
        napi_status status = napi_wrap(
            env, obj, entity.release(),
            [](napi_env env, void *data, void *hint) {
                std::unique_ptr<T>(static_cast<T *>(data));
            },
            nullptr, nullptr);
        return status == napi_ok;
    }

    template <class T> static T *RemoveEntityOfFinal(napi_env env, napi_value objStat)
    {
        std::lock_guard<std::mutex> lock(wrapLock);
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
    NClass() : addCleanHook(false) {};
    ~NClass() = default;
    static NClass &GetInstance();
    std::map<std::string, napi_ref> exClassMap;
    std::mutex exClassMapLock;
    static std::mutex wrapLock;
    bool addCleanHook;
    static bool wrapRelease;
};
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMGMT_LIBN_N_CLASS_H
