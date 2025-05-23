/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef FILEMGMT_LIBN_N_VAL_H
#define FILEMGMT_LIBN_N_VAL_H

#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

#include "n_napi.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
class NVal final {
public:
    NVal() = default;
    NVal(napi_env nEnv, napi_value nVal);
    NVal(const NVal &) = default;
    NVal &operator=(const NVal &) = default;
    virtual ~NVal() = default;

    /* NOTE! env_ and val_ is LIKELY to be null */
    napi_env env_ = nullptr;
    napi_value val_ = nullptr;

    explicit operator bool() const;
    bool TypeIs(napi_valuetype expType) const;
    bool TypeIsError(bool checkErrno = false) const;

    /* SHOULD ONLY BE USED FOR EXPECTED TYPE */
    std::tuple<bool, std::unique_ptr<char[]>, size_t> ToUTF8String() const;
    std::tuple<bool, std::unique_ptr<char[]>, size_t> ToUTF8String(std::string defaultValue) const;
    std::tuple<bool, std::unique_ptr<char[]>, size_t> ToUTF16String() const;
    std::tuple<bool, std::unique_ptr<char[]>, size_t> ToUTF8StringPath() const;
    std::tuple<bool, void *> ToPointer() const;
    std::tuple<bool, bool> ToBool() const;
    std::tuple<bool, bool> ToBool(bool defaultValue) const;
    std::tuple<bool, int32_t> ToInt32() const;
    std::tuple<bool, int32_t> ToInt32(int32_t defaultValue) const;
    std::tuple<bool, int64_t> ToInt64() const;
    std::tuple<bool, int64_t> ToInt64(int64_t defaultValue) const;
    std::tuple<bool, void *, size_t> ToArraybuffer() const;
    std::tuple<bool, void *, size_t> ToTypedArray() const;
    std::tuple<bool, std::vector<std::string>, uint32_t> ToStringArray();
    std::tuple<bool, uint64_t, bool> ToUint64() const;
    std::tuple<bool, uint32_t> ToUint32() const;
    std::tuple<bool, double> ToDouble() const;

    /* Static helpers to create js objects */
    static NVal CreateUndefined(napi_env env);
    static NVal CreateBigInt64(napi_env env, int64_t val);
    static NVal CreateBigIntUint64(napi_env env, uint64_t val);
    static NVal CreateInt64(napi_env env, int64_t val);
    static NVal CreateInt32(napi_env env, int32_t val);
    static NVal CreateUint32(napi_env env, int32_t val);
    static NVal CreateObject(napi_env env);
    static NVal CreateBool(napi_env env, bool val);
    static NVal CreateUTF8String(napi_env env, std::string str);
    static NVal CreateUTF8String(napi_env env, const char *str, ssize_t len);
    static NVal CreateUint8Array(napi_env env, void *buf, size_t bufLen);
    static NVal CreateArrayString(napi_env env, std::vector<std::string> strs);
    static std::tuple<NVal, void *> CreateArrayBuffer(napi_env env, size_t len);

    /* SHOULD ONLY BE USED FOR OBJECT */
    bool HasProp(std::string propName) const;
#ifdef WIN_PLATFORM
    NVal GetPropValue(std::string propName) const;
#else
    NVal GetProp(std::string propName) const;
#endif
    bool AddProp(std::vector<napi_property_descriptor> &&propVec) const;
    bool AddProp(std::string propName, napi_value nVal) const;

    /* Static helpers to create prop of js objects */
    static napi_property_descriptor DeclareNapiProperty(const char *name, napi_value val);
    static napi_property_descriptor DeclareNapiStaticProperty(const char *name, napi_value val);
    static napi_property_descriptor DeclareNapiFunction(const char *name, napi_callback func);
    static napi_property_descriptor DeclareNapiStaticFunction(const char *name, napi_callback func);
    static napi_property_descriptor DeclareNapiGetter(const char *name, napi_callback getter);
    static napi_property_descriptor DeclareNapiSetter(const char *name, napi_callback setter);
    static inline napi_property_descriptor
        DeclareNapiGetterSetter(const char *name, napi_callback getter, napi_callback setter);
};
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMGMT_LIBN_N_VAL_H
