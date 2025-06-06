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

#ifndef N_REF_H
#define N_REF_H

#include "n_val.h"

namespace OHOS {
namespace DistributedFS {
class NRef {
public:
    NRef();
    explicit NRef(NVal val);
    ~NRef();

    explicit operator bool() const;
    NVal Deref(napi_env env);

private:
    napi_env env_ = nullptr;
    napi_ref ref_ = nullptr;
};
} // namespace DistributedFS
} // namespace OHOS
#endif // N_REF_H