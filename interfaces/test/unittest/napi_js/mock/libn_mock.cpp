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

#include "libn_mock.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace OHOS::FileManagement::ModuleFileIO;

thread_local std::shared_ptr<LibnMock> LibnMock::libnMock = nullptr;
thread_local bool LibnMock::mockable = false;

std::shared_ptr<LibnMock> LibnMock::GetMock()
{
    if (libnMock == nullptr) {
        libnMock = std::make_shared<LibnMock>();
    }
    return libnMock;
}

void LibnMock::EnableMock()
{
    mockable = true;
}

void LibnMock::DisableMock()
{
    libnMock = nullptr;
    mockable = false;
}

bool LibnMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

#ifdef __cplusplus
using namespace OHOS::FileManagement::ModuleFileIO::Test;

extern "C" {
napi_status napi_unwrap(napi_env env, napi_value js_object, void **result)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->napi_unwrap(env, js_object, result);
    }

    static napi_status (*realNapi)(napi_env, napi_value, void **) = []() {
        auto func = (napi_status(*)(napi_env, napi_value, void **))dlsym(RTLD_NEXT, "napi_unwrap");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_unwrap: " << dlerror();
        }
        return func;
    }();

    if (!realNapi) {
        return napi_ok;
    }

    return realNapi(env, js_object, result);
}

napi_status napi_remove_wrap(napi_env env, napi_value js_object, void **result)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->napi_remove_wrap(env, js_object, result);
    }

    static napi_status (*realNapi)(napi_env, napi_value, void **) = []() {
        auto func = (napi_status(*)(napi_env, napi_value, void **))dlsym(RTLD_NEXT, "napi_remove_wrap");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_remove_wrap: " << dlerror();
        }
        return func;
    }();

    if (!realNapi) {
        return napi_ok;
    }

    return realNapi(env, js_object, result);
}

napi_status napi_create_array(napi_env env, napi_value *result)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->napi_create_array(env, result);
    }

    static napi_status (*realNapiCreateArray)(napi_env, napi_value *) = []() {
        auto func = (napi_status(*)(napi_env, napi_value *))dlsym(RTLD_NEXT, "napi_create_array");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real napi_create_array: " << dlerror();
        }
        return func;
    }();

    if (!realNapiCreateArray) {
        return napi_ok;
    }

    return realNapiCreateArray(env, result);
}

} // extern "C"

bool NFuncArg::InitArgs(std::function<bool()> argcChecker)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->InitArgs(argcChecker);
    }

    static bool (*realInitArgs)(std::function<bool()>) = []() {
        auto func = (bool (*)(std::function<bool()>))dlsym(RTLD_NEXT, "InitArgs");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real InitArgs: " << dlerror();
        }
        return func;
    }();

    if (!realInitArgs) {
        return false;
    }

    return realInitArgs(argcChecker);
}

bool NFuncArg::InitArgs(size_t argc)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->InitArgs(argc);
    }

    static bool (*realInitArgs)(size_t) = []() {
        auto func = (bool (*)(size_t))dlsym(RTLD_NEXT, "InitArgs");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real InitArgs: " << dlerror();
        }
        return func;
    }();

    if (!realInitArgs) {
        return false;
    }

    return realInitArgs(argc);
}

bool NFuncArg::InitArgs(size_t minArgc, size_t maxArgc)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->InitArgs(minArgc, maxArgc);
    }

    static bool (*realInitArgs)(size_t, size_t) = []() {
        auto func = (bool (*)(size_t, size_t))dlsym(RTLD_NEXT, "InitArgs");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real InitArgs: " << dlerror();
        }
        return func;
    }();

    if (!realInitArgs) {
        return false;
    }

    return realInitArgs(minArgc, maxArgc);
}

size_t NFuncArg::GetArgc() const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->GetArgc();
    }

    static size_t (*realGetArgc)() = []() {
        auto func = (size_t(*)())dlsym(RTLD_NEXT, "GetArgc");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real GetArgc: " << dlerror();
        }
        return func;
    }();

    if (!realGetArgc) {
        return -1;
    }

    return realGetArgc();
}

napi_value NFuncArg::GetThisVar() const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->GetThisVar();
    }

    static napi_value (*realGetThisVar)() = []() {
        auto func = (napi_value(*)())dlsym(RTLD_NEXT, "GetThisVar");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real GetThisVar: " << dlerror();
        }
        return func;
    }();

    if (!realGetThisVar) {
        return nullptr;
    }

    return realGetThisVar();
}

napi_value NFuncArg::GetArg(size_t argPos) const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->GetArg(argPos);
    }

    static napi_value (*realGetArg)(size_t) = []() {
        auto func = (napi_value(*)(size_t))dlsym(RTLD_NEXT, "GetArg");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real GetArg: " << dlerror();
        }
        return func;
    }();

    if (!realGetArg) {
        return nullptr;
    }

    return realGetArg(argPos);
}

void NError::ThrowErr(napi_env env)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ThrowErr(env);
    }

    static void (*realThrowErr)(napi_env) = []() {
        auto func = (void (*)(napi_env))dlsym(RTLD_NEXT, "ThrowErr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ThrowErr: " << dlerror();
        }
        return func;
    }();

    if (!realThrowErr) {
        return;
    }

    return realThrowErr(env);
}

void NError::ThrowErr(napi_env env, int errCode)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ThrowErr(env, errCode);
    }

    static void (*realThrowErr)(napi_env, int) = []() {
        auto func = (void (*)(napi_env, int))dlsym(RTLD_NEXT, "ThrowErr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ThrowErr: " << dlerror();
        }
        return func;
    }();

    if (!realThrowErr) {
        return;
    }

    return realThrowErr(env, errCode);
}

void NError::ThrowErr(napi_env env, std::string errMsg)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ThrowErr(env, errMsg);
    }

    static void (*realThrowErr)(napi_env, std::string) = []() {
        auto func = (void (*)(napi_env, std::string))dlsym(RTLD_NEXT, "ThrowErr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ThrowErr: " << dlerror();
        }
        return func;
    }();

    if (!realThrowErr) {
        return;
    }

    return realThrowErr(env, errMsg);
}

void NError::ThrowErrAddData(napi_env env, int errCode, napi_value data)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ThrowErrAddData(env, errCode, data);
    }

    static void (*realThrowErr)(napi_env, int, napi_value) = []() {
        auto func = (void (*)(napi_env, int, napi_value))dlsym(RTLD_NEXT, "ThrowErrAddData");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ThrowErrAddData: " << dlerror();
        }
        return func;
    }();

    if (!realThrowErr) {
        return;
    }

    return realThrowErr(env, errCode, data);
}

std::tuple<bool, std::unique_ptr<char[]>, size_t> NVal::ToUTF8String() const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToUTF8String();
    }

    static std::tuple<bool, std::unique_ptr<char[]>, size_t> (*realToUTF8String)() = []() {
        auto func = (std::tuple<bool, std::unique_ptr<char[]>, size_t>(*)())dlsym(RTLD_NEXT, "ToUTF8String");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToUTF8String: " << dlerror();
        }
        return func;
    }();

    if (!realToUTF8String) {
        return { false, nullptr, -1 };
    }

    return realToUTF8String();
}

std::tuple<bool, std::unique_ptr<char[]>, size_t> NVal::ToUTF8String(std::string defaultValue) const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToUTF8String(defaultValue);
    }

    static std::tuple<bool, std::unique_ptr<char[]>, size_t> (*realToUTF8String)(std::string) = []() {
        auto func = (std::tuple<bool, std::unique_ptr<char[]>, size_t>(*)(std::string))dlsym(RTLD_NEXT, "ToUTF8String");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToUTF8String: " << dlerror();
        }
        return func;
    }();

    if (!realToUTF8String) {
        return { false, nullptr, -1 };
    }

    return realToUTF8String(defaultValue);
}

std::tuple<bool, std::unique_ptr<char[]>, size_t> NVal::ToUTF8StringPath() const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToUTF8StringPath();
    }

    static std::tuple<bool, std::unique_ptr<char[]>, size_t> (*realToUTF8StringPath)() = []() {
        auto func = (std::tuple<bool, std::unique_ptr<char[]>, size_t>(*)())dlsym(RTLD_NEXT, "ToUTF8StringPath");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToUTF8StringPath: " << dlerror();
        }
        return func;
    }();

    if (!realToUTF8StringPath) {
        return { false, nullptr, -1 };
    }

    return realToUTF8StringPath();
}

std::tuple<bool, bool> NVal::ToBool() const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToBool();
    }

    static std::tuple<bool, bool> (*realToBool)() = []() {
        auto func = (std::tuple<bool, bool>(*)())dlsym(RTLD_NEXT, "ToBool");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToBool: " << dlerror();
        }
        return func;
    }();

    if (!realToBool) {
        return { false, false };
    }

    return realToBool();
}

std::tuple<bool, bool> NVal::ToBool(bool defaultValue) const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToBool(defaultValue);
    }

    static std::tuple<bool, bool> (*realToBool)(bool) = []() {
        auto func = (std::tuple<bool, bool>(*)(bool))dlsym(RTLD_NEXT, "ToBool");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToBool: " << dlerror();
        }
        return func;
    }();

    if (!realToBool) {
        return { false, false };
    }

    return realToBool(defaultValue);
}

std::tuple<bool, int32_t> NVal::ToInt32() const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToInt32();
    }

    static std::tuple<bool, int32_t> (*realToInt32)() = []() {
        auto func = (std::tuple<bool, int32_t>(*)())dlsym(RTLD_NEXT, "ToInt32");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToInt32: " << dlerror();
        }
        return func;
    }();

    if (!realToInt32) {
        return { false, 0 };
    }

    return realToInt32();
}

std::tuple<bool, int32_t> NVal::ToInt32(int32_t defaultValue) const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToInt32(defaultValue);
    }

    static std::tuple<bool, int32_t> (*realToInt32)(int32_t) = []() {
        auto func = (std::tuple<bool, int32_t>(*)(int32_t))dlsym(RTLD_NEXT, "ToInt32");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToInt32: " << dlerror();
        }
        return func;
    }();

    if (!realToInt32) {
        return { false, 0 };
    }

    return realToInt32(defaultValue);
}

std::tuple<bool, int64_t> NVal::ToInt64() const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToInt64();
    }

    static std::tuple<bool, int64_t> (*realToInt64)() = []() {
        auto func = (std::tuple<bool, int64_t>(*)())dlsym(RTLD_NEXT, "ToInt64");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToInt64: " << dlerror();
        }
        return func;
    }();

    if (!realToInt64) {
        return { false, 0 };
    }

    return realToInt64();
}

std::tuple<bool, int64_t> NVal::ToInt64(int64_t defaultValue) const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToInt64(defaultValue);
    }

    static std::tuple<bool, int64_t> (*realToInt64)(int64_t) = []() {
        auto func = (std::tuple<bool, int64_t>(*)(int64_t))dlsym(RTLD_NEXT, "ToInt64");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToInt64: " << dlerror();
        }
        return func;
    }();

    if (!realToInt64) {
        return { false, 0 };
    }

    return realToInt64(defaultValue);
}

std::tuple<bool, double> NVal::ToDouble() const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->ToDouble();
    }

    static std::tuple<bool, double> (*realToDouble)() = []() {
        auto func = (std::tuple<bool, double>(*)())dlsym(RTLD_NEXT, "ToDouble");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ToDouble: " << dlerror();
        }
        return func;
    }();

    if (!realToDouble) {
        return { false, 0 };
    }

    return realToDouble();
}

bool NVal::HasProp(std::string propName) const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->HasProp(propName);
    }

    static bool (*realHasProp)(std::string) = []() {
        auto func = (bool (*)(std::string))dlsym(RTLD_NEXT, "HasProp");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real HasProp: " << dlerror();
        }
        return func;
    }();

    if (!realHasProp) {
        return false;
    }

    return realHasProp(propName);
}

NVal NVal::GetProp(std::string propName) const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->GetProp(propName);
    }

    static NVal (*realGetProp)(std::string) = []() {
        auto func = (NVal(*)(std::string))dlsym(RTLD_NEXT, "GetProp");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real GetProp: " << dlerror();
        }
        return func;
    }();

    if (!realGetProp) {
        napi_env env = reinterpret_cast<napi_env>(0x1000);
        napi_value val = reinterpret_cast<napi_value>(0x1000);
        NVal myOp(env, val);
        return myOp;
    }

    return realGetProp(propName);
}

bool NVal::TypeIs(napi_valuetype expType) const
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->TypeIs(expType);
    }

    static bool (*realTypeIs)(napi_valuetype) = []() {
        auto func = (bool (*)(napi_valuetype))dlsym(RTLD_NEXT, "TypeIs");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real TypeIs: " << dlerror();
        }
        return func;
    }();

    if (!realTypeIs) {
        return false;
    }

    return realTypeIs(expType);
}

NVal NVal::CreateUndefined(napi_env env)
{
    if (LibnMock::IsMockable()) {
        return LibnMock::GetMock()->CreateUndefined(env);
    }

    static NVal (*realCreateUndefined)(napi_env) = []() {
        auto func = (NVal(*)(napi_env))dlsym(RTLD_NEXT, "CreateUndefined");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real CreateUndefined: " << dlerror();
        }
        return func;
    }();

    if (!realCreateUndefined) {
        return { nullptr, nullptr };
    }

    return realCreateUndefined(env);
}
#endif