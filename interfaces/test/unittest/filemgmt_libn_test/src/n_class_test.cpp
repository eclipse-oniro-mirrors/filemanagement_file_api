/*
 * Copyright (C) 2023-2026 Huawei Device Co., Ltd.
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

#include "n_class.h"

#include <string>
#include <thread>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "napi_mock.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
namespace Test {
using namespace std;

class NClassTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void NClassTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "NClassTest");
    NapiMock::EnableMock();
}

void NClassTest::TearDownTestSuite()
{
    NapiMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void NClassTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void NClassTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: NClassTest_DefineClass_001
 * @tc.desc: Test function of NClass::DefineClass interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_DefineClass_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_DefineClass_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_DefineClass_001";
    napi_callback callback = reinterpret_cast<napi_callback>(0x1122);
    vector<napi_property_descriptor> props = {};

    // Set mock behaviors
    napi_value expectedClass = reinterpret_cast<napi_value>(0x1234);
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_define_class(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_,
                               testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(expectedClass), testing::Return(napi_ok)));

    // Do testing
    auto [succ, classValue] = NClass::DefineClass(env, className, callback, std::move(props));

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(succ);
    EXPECT_EQ(classValue, expectedClass);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_DefineClass_001";
}

/**
 * @tc.name: NClassTest_DefineClass_002
 * @tc.desc: Test function of NClass::DefineClass interface for FAILURE when napi_define_class fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_DefineClass_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_DefineClass_002";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_DefineClass_002";
    napi_callback callback = reinterpret_cast<napi_callback>(0x1122);
    vector<napi_property_descriptor> props = {};

    // Set mock behaviors
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_define_class(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_,
                               testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    auto [succ, classValue] = NClass::DefineClass(env, className, callback, std::move(props));

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_FALSE(succ);
    EXPECT_EQ(classValue, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_DefineClass_002";
}

/**
 * @tc.name: NClassTest_SaveClass_001
 * @tc.desc: Test function of NClass::SaveClass interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SaveClass_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SaveClass_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_SaveClass_001";

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    EXPECT_CALL(*napiMock, napi_add_env_cleanup_hook(testing::_, testing::_, testing::Ne(env)))
        .WillOnce(testing::Return(napi_ok));

    // Do testing
    bool succ = NClass::SaveClass(env, className, classValue);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(succ);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SaveClass_001";
}

/**
 * @tc.name: NClassTest_SaveClass_002
 * @tc.desc: Test function of NClass::SaveClass interface for FAILURE when napi_create_reference fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SaveClass_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SaveClass_002";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_SaveClass_002";
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);

    // Set mock behaviors
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    bool succ = NClass::SaveClass(env, className, classValue);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_FALSE(succ);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SaveClass_002";
}

/**
 * @tc.name: NClassTest_CleanClass_001
 * @tc.desc: Test function of NClass::CleanClass interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_CleanClass_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_CleanClass_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_CleanClass_001";

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    // Cases share one thread_local instance: one ref left by NClassTest_SaveClass_001 plus this one
    EXPECT_CALL(*napiMock, napi_delete_reference(testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(napi_ok));

    // Do testing
    bool succ = NClass::SaveClass(env, className, classValue);
    EXPECT_TRUE(succ);
    NClass::CleanClass(&NClass::GetInstance());

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_CleanClass_001";
}

/**
 * @tc.name: NClassTest_InstantiateClass_001
 * @tc.desc: Test function of NClass::InstantiateClass interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_InstantiateClass_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_InstantiateClass_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_InstantiateClass_001";
    vector<napi_value> emptyArgs = {};

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    EXPECT_CALL(*napiMock, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));

    napi_value expectedInstance = reinterpret_cast<napi_value>(0x3456);
    EXPECT_CALL(*napiMock, napi_new_instance(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<4>(expectedInstance), testing::Return(napi_ok)));

    // Prepare instance
    bool succ = NClass::SaveClass(env, className, classValue);
    EXPECT_TRUE(succ);

    // Do testing
    auto instance = NClass::InstantiateClass(env, className, emptyArgs);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_EQ(instance, expectedInstance);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_InstantiateClass_001";
}

/**
 * @tc.name: NClassTest_InstantiateClass_002
 * @tc.desc: Test function of NClass::InstantiateClass interface for FAILURE when class hasn't been saved yet.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_InstantiateClass_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_InstantiateClass_002";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_InstantiateClass_002";
    vector<napi_value> emptyArgs = {};

    // Do testing
    auto instance = NClass::InstantiateClass(env, className, emptyArgs);

    // Verify results
    EXPECT_EQ(instance, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_InstantiateClass_002";
}

/**
 * @tc.name: NClassTest_InstantiateClass_003
 * @tc.desc: Test function of NClass::InstantiateClass interface for FAILURE when napi_get_reference_value fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_InstantiateClass_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_InstantiateClass_003";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_InstantiateClass_003";
    vector<napi_value> emptyArgs = {};

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    EXPECT_CALL(*napiMock, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_invalid_arg));

    // Prepare instance
    bool succ = NClass::SaveClass(env, className, classValue);
    EXPECT_TRUE(succ);

    // Do testing
    auto instance = NClass::InstantiateClass(env, className, emptyArgs);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_EQ(instance, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_InstantiateClass_003";
}

/**
 * @tc.name: NClassTest_InstantiateClass_004
 * @tc.desc: Test function of NClass::InstantiateClass interface for FAILURE when napi_new_instance fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_InstantiateClass_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_InstantiateClass_004";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_InstantiateClass_004";
    vector<napi_value> emptyArgs = {};

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    EXPECT_CALL(*napiMock, napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));

    EXPECT_CALL(*napiMock, napi_new_instance(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Prepare instance
    bool succ = NClass::SaveClass(env, className, classValue);
    EXPECT_TRUE(succ);

    // Do testing
    auto instance = NClass::InstantiateClass(env, className, emptyArgs);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_EQ(instance, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_InstantiateClass_004";
}

/**
 * @tc.name: NClassTest_GetEntityOf_001
 * @tc.desc: Test function of NClass::GetEntityOf interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_GetEntityOf_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_GetEntityOf_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);
    void *expectedEntity = reinterpret_cast<void *>(0x23456);

    // Set mock behaviors
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(expectedEntity), testing::Return(napi_ok)));

    // Do testing
    auto entity = NClass::GetEntityOf<string>(env, object);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_EQ(entity, expectedEntity);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_GetEntityOf_001";
}

/**
 * @tc.name: NClassTest_GetEntityOf_002
 * @tc.desc: Test function of NClass::GetEntityOf interface for FAILURE when arg env nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_GetEntityOf_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_GetEntityOf_002";
    // Prepare test parameters
    napi_value object = reinterpret_cast<napi_value>(0x1234);

    // Do testing
    auto entity = NClass::GetEntityOf<string>(nullptr, object);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_GetEntityOf_002";
}

/**
 * @tc.name: NClassTest_GetEntityOf_003
 * @tc.desc: Test function of NClass::GetEntityOf interface for FAILURE when arg objStat nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_GetEntityOf_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_GetEntityOf_003";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);

    // Do testing
    auto entity = NClass::GetEntityOf<string>(env, nullptr);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_GetEntityOf_003";
}

/**
 * @tc.name: NClassTest_GetEntityOf_004
 * @tc.desc: Test function of NClass::GetEntityOf interface for FAILURE when napi_unwrap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_GetEntityOf_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_GetEntityOf_004";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);

    // Set mock behaviors
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    auto entity = NClass::GetEntityOf<string>(env, object);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_GetEntityOf_004";
}

/**
 * @tc.name: NClassTest_SetEntityFor_001
 * @tc.desc: Test function of NClass::SetEntityFor interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SetEntityFor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SetEntityFor_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);
    string entity = "NClassTest_SetEntityFor_001";

    // Set mock behaviors
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_wrap(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));

    // Do testing
    bool result = NClass::SetEntityFor<string>(env, object, make_unique<string>(entity));

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SetEntityFor_001";
}

/**
 * @tc.name: NClassTest_SetEntityFor_002
 * @tc.desc: Test function of NClass::SetEntityFor interface for FAILURE when napi_wrap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SetEntityFor_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SetEntityFor_002";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);
    string entity = "NClassTest_SetEntityFor_001";

    // Set mock behaviors
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_wrap(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    bool result = NClass::SetEntityFor<string>(env, object, make_unique<string>(entity));

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SetEntityFor_002";
}

/**
 * @tc.name: NClassTest_RemoveEntityOfFinal_001
 * @tc.desc: Test function of NClass::RemoveEntityOfFinal interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_RemoveEntityOfFinal_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_RemoveEntityOfFinal_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);
    void *expectedEntity = reinterpret_cast<void *>(0x23456);

    // Set mock behaviors
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_remove_wrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(expectedEntity), testing::Return(napi_ok)));

    // Do testing
    auto entity = NClass::RemoveEntityOfFinal<string>(env, object);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_EQ(entity, expectedEntity);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_RemoveEntityOfFinal_001";
}

/**
 * @tc.name: NClassTest_RemoveEntityOfFinal_002
 * @tc.desc: Test function of NClass::RemoveEntityOfFinal interface for FAILURE when arg env nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_RemoveEntityOfFinal_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_RemoveEntityOfFinal_002";
    // Prepare test parameters
    napi_value object = reinterpret_cast<napi_value>(0x1234);

    // Do testing
    auto entity = NClass::RemoveEntityOfFinal<string>(nullptr, object);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_RemoveEntityOfFinal_002";
}

/**
 * @tc.name: NClassTest_RemoveEntityOfFinal_003
 * @tc.desc: Test function of NClass::RemoveEntityOfFinal interface for FAILURE when arg objStat nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_RemoveEntityOfFinal_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_RemoveEntityOfFinal_003";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);

    // Do testing
    auto entity = NClass::RemoveEntityOfFinal<string>(env, nullptr);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_RemoveEntityOfFinal_003";
}

/**
 * @tc.name: NClassTest_RemoveEntityOfFinal_004
 * @tc.desc: Test function of NClass::RemoveEntityOfFinal interface for FAILURE when napi_remove_wrap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_RemoveEntityOfFinal_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_RemoveEntityOfFinal_004";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);

    // Set mock behaviors
    auto napiMock = NapiMock::GetMock();
    EXPECT_CALL(*napiMock, napi_remove_wrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    auto entity = NClass::RemoveEntityOfFinal<string>(env, object);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_RemoveEntityOfFinal_004";
}

/**
 * @tc.name: NClassTest_SaveClass_HookArgUnique
 * @tc.desc: Test function of NClass::SaveClass interface for SUCCESS when cleanup hook arg is unique.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SaveClass_HookArgUnique, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SaveClass_HookArgUnique";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x2000);
    string className = "NClassTest_SaveClass_HookArgUnique";

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    auto napiMock = NapiMock::GetMock();

    // Reset the shared thread_local instance left by earlier cases
    EXPECT_CALL(*napiMock, napi_delete_reference(testing::_, testing::_))
        .WillRepeatedly(testing::Return(napi_ok));
    NClass::CleanClass(&NClass::GetInstance());
    testing::Mock::VerifyAndClearExpectations(napiMock.get());

    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    EXPECT_CALL(*napiMock,
        napi_add_env_cleanup_hook(testing::_, testing::_,
            testing::AllOf(testing::Ne(env), testing::Eq(static_cast<void *>(&NClass::GetInstance())))))
        .WillOnce(testing::Return(napi_ok));

    // Do testing
    bool succ = NClass::SaveClass(env, className, classValue);

    // Verify results
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(succ);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SaveClass_HookArgUnique";
}

/**
 * @tc.name: NClassTest_CleanClass_ResetForNewEnv
 * @tc.desc: Test function of NClass interface for SUCCESS when a new env re-registers the hook and
 *           wrapReleased is reset after CleanClass.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_CleanClass_ResetForNewEnv, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_CleanClass_ResetForNewEnv";
    // Prepare test parameters
    napi_env firstEnv = reinterpret_cast<napi_env>(0x2100);
    napi_env secondEnv = reinterpret_cast<napi_env>(0x2200);
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    auto napiMock = NapiMock::GetMock();

    // Reset the shared thread_local instance left by earlier cases
    EXPECT_CALL(*napiMock, napi_delete_reference(testing::_, testing::_))
        .WillRepeatedly(testing::Return(napi_ok));
    NClass::CleanClass(&NClass::GetInstance());
    testing::Mock::VerifyAndClearExpectations(napiMock.get());

    // Register on the first env and tear it down via CleanClass
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));
    EXPECT_CALL(*napiMock, napi_add_env_cleanup_hook(firstEnv, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    bool succ = NClass::SaveClass(firstEnv, "NClassTest_CleanClass_ResetForNewEnv_1", classValue);
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(succ);

    EXPECT_CALL(*napiMock, napi_delete_reference(testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    NClass::CleanClass(&NClass::GetInstance());
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(NClass::GetInstance().wrapReleased);

    // SaveClass on the new env re-registers the hook once per env and resets wrapReleased
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));
    EXPECT_CALL(*napiMock, napi_add_env_cleanup_hook(secondEnv, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    succ = NClass::SaveClass(secondEnv, "NClassTest_CleanClass_ResetForNewEnv_2", classValue);
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(succ);
    EXPECT_FALSE(NClass::GetInstance().wrapReleased);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_CleanClass_ResetForNewEnv";
}

/**
 * @tc.name: NClassTest_CleanClass_NullArg
 * @tc.desc: Test function of NClass::CleanClass interface for SUCCESS when arg is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_CleanClass_NullArg, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_CleanClass_NullArg";

    // Do testing
    NClass::CleanClass(nullptr);

    // Verify results: safe return without any napi call or crash
    SUCCEED();
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_CleanClass_NullArg";
}

/**
 * @tc.name: NClassTest_SaveClass_HookFailRetry
 * @tc.desc: Test function of NClass::SaveClass interface for SUCCESS when cleanup hook registration
 *           fails once and is retried on the next SaveClass with the same env.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SaveClass_HookFailRetry, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SaveClass_HookFailRetry";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x2300);
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    auto napiMock = NapiMock::GetMock();

    // Reset the shared thread_local instance left by earlier cases
    EXPECT_CALL(*napiMock, napi_delete_reference(testing::_, testing::_))
        .WillRepeatedly(testing::Return(napi_ok));
    NClass::CleanClass(&NClass::GetInstance());
    testing::Mock::VerifyAndClearExpectations(napiMock.get());

    // First SaveClass: hook registration fails, env_ keeps the old value for retry
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));
    EXPECT_CALL(*napiMock, napi_add_env_cleanup_hook(env, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));
    bool succ = NClass::SaveClass(env, "NClassTest_SaveClass_HookFailRetry_1", classValue);
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(succ);

    // Second SaveClass with the same env: registration is retried and succeeds
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));
    EXPECT_CALL(*napiMock, napi_add_env_cleanup_hook(env, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));
    succ = NClass::SaveClass(env, "NClassTest_SaveClass_HookFailRetry_2", classValue);
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(succ);

    // Third SaveClass with the same env: no more registration once it has succeeded
    EXPECT_CALL(*napiMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));
    EXPECT_CALL(*napiMock, napi_add_env_cleanup_hook(testing::_, testing::_, testing::_)).Times(0);
    succ = NClass::SaveClass(env, "NClassTest_SaveClass_HookFailRetry_3", classValue);
    testing::Mock::VerifyAndClearExpectations(napiMock.get());
    EXPECT_TRUE(succ);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SaveClass_HookFailRetry";
}

/**
 * @tc.name: NClassTest_CleanClass_NullEnv
 * @tc.desc: Test function of NClass::CleanClass interface for SUCCESS when the instance has no env recorded.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_CleanClass_NullEnv, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_CleanClass_NullEnv";

    // A fresh thread owns a fresh thread_local NClass instance whose env_ is nullptr;
    // the deep path would flip wrapReleased to true before anything else
    bool wrapReleasedObserved = true;
    std::thread freshThread([&wrapReleasedObserved] {
        NClass &nClass = NClass::GetInstance();
        NClass::CleanClass(&nClass);
        wrapReleasedObserved = nClass.wrapReleased;
    });
    freshThread.join();

    // Early return on null env keeps wrapReleased false, without crash or hang
    EXPECT_FALSE(wrapReleasedObserved);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_CleanClass_NullEnv";
}

} // namespace Test
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS