/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "randomaccessfile_n_exporter.h"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "common_func.h"
#include "fcntl_mock.h"
#include "fdsan_mock.h"
#include "libn_mock.h"
#include "randomaccessfile_entity.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class RandomAccessFileNExporterMockTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();

protected:
    unique_ptr<RandomAccessFileEntity> rafEntity;
};

void RandomAccessFileNExporterMockTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "RandomAccessFileNExporterMockTest");
    LibnMock::EnableMock();
    FdsanMock::EnableMock();
    UvFsMock::EnableMock();
}

void RandomAccessFileNExporterMockTest::TearDownTestSuite(void)
{
    LibnMock::DisableMock();
    FdsanMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void RandomAccessFileNExporterMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    rafEntity = make_unique<RandomAccessFileEntity>();
    const int fdValue = 3;
    const bool isClosed = false;
    rafEntity->fd = make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    rafEntity->filePointer = 0;
}

void RandomAccessFileNExporterMockTest::TearDown(void)
{
    LibnMock::GetMock()->ResetErrState();
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_001
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for SUCCESS
 * when fdsan_close_with_tag succeeds.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, napi_remove_wrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, CreateUndefined(_)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*libnMock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_001";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_002
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for FAILURE
 * when fdsan_close_with_tag fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EBADFD, -1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    libnMock->VerifyAndClearErr(13900039, "File descriptor in bad state");
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_002";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_003
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for FAILURE
 * when napi_unwrap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_003";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_004
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for FAILURE
 * when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_004";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_004";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_005
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for FAILURE
 * when GetThisVar returns nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_005";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nullptr));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_005";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_006
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for FAILURE
 * when GetThisVar returns nullptr on second call.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_006";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv)).WillOnce(Return(nullptr));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_006";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_007
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for FAILURE
 * when napi_remove_wrap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_007";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, napi_remove_wrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_007";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_008
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for SUCCESS
 * when uv_fs_close succeeds.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_008";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    unique_ptr<RandomAccessFileEntity> rafEntity_;
    rafEntity_ = make_unique<RandomAccessFileEntity>();
    const int fdValue = FD_SAN_OVERFLOW_END;
    const bool isClosed = false;
    rafEntity_->fd = make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    rafEntity_->filePointer = 0;

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity_.get())), Return(napi_ok)));
    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*libnMock, napi_remove_wrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, CreateUndefined(_)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*libnMock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_008";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_009
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for FAILURE
 * when uv_fs_close fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_009";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    unique_ptr<RandomAccessFileEntity> rafEntity_;
    rafEntity_ = make_unique<RandomAccessFileEntity>();
    const int fdValue = FD_SAN_OVERFLOW_END;
    const bool isClosed = false;
    rafEntity_->fd = make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    rafEntity_->filePointer = 0;

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity_.get())), Return(napi_ok)));
    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    libnMock->VerifyAndClearErr(13900001, "Operation not permitted");
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_009";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_010
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for SUCCESS
 * when fdsan tag matches fileTag (tag > 0 and tag == fileTag).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_010";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    uint64_t fileTag = static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(rafEntity.get()));
    const int fdValue = 3;
    CommonFunc::SetFdTag(fdValue, fileTag);

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, napi_remove_wrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, CreateUndefined(_)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*libnMock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    CommonFunc::SetFdTag(fdValue, 0);
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_010";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_011
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for SUCCESS
 * when fdsan tag does not match fileTag (tag > 0 and tag != fileTag).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_011";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    const int fdValue = 3;
    CommonFunc::SetFdTag(fdValue, 1);

    auto libnMock = LibnMock::GetMock();
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*libnMock, napi_remove_wrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, CreateUndefined(_)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    EXPECT_CALL(*libnMock, ThrowErrWithMsg(testing::_, testing::_)).Times(0);

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
    CommonFunc::SetFdTag(fdValue, 0);
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_011";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetFD_001
 * @tc.desc: Test function of RandomAccessFileNExporter::GetFD interface for FAILURE
 * when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetFD_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetFD(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetFD_002
 * @tc.desc: Test function of RandomAccessFileNExporter::GetFD interface for FAILURE
 * when GetRAFEntity fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetFD_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetFD(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetFD_003
 * @tc.desc: Test function of RandomAccessFileNExporter::GetFD interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetFD_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, CreateInt32(_, _)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    auto res = RandomAccessFileNExporter::GetFD(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetFPointer_001
 * @tc.desc: Test function of RandomAccessFileNExporter::GetFPointer interface for FAILURE
 * when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetFPointer_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetFPointer(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetFPointer_002
 * @tc.desc: Test function of RandomAccessFileNExporter::GetFPointer interface for FAILURE
 * when GetRAFEntity fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetFPointer_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetFPointer(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetFPointer_003
 * @tc.desc: Test function of RandomAccessFileNExporter::GetFPointer interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetFPointer_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };
    rafEntity->filePointer = 42;
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, CreateInt64(_, _)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    auto res = RandomAccessFileNExporter::GetFPointer(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_SetFilePointerSync_001
 * @tc.desc: Test function of SetFilePointerSync for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_SetFilePointerSync_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::SetFilePointerSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_SetFilePointerSync_002
 * @tc.desc: Test function of SetFilePointerSync for FAILURE when GetRAFEntity fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_SetFilePointerSync_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::SetFilePointerSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_SetFilePointerSync_003
 * @tc.desc: Test function of SetFilePointerSync for FAILURE when ToInt64 fails (!succ=true).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_SetFilePointerSync_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg = reinterpret_cast<napi_value>(0x1300);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg));
    EXPECT_CALL(*libnMock, ToInt64()).WillOnce(Return(make_tuple(false, 0)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::SetFilePointerSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_SetFilePointerSync_004
 * @tc.desc: Test function of SetFilePointerSync for SUCCESS when fp < 0 (less than).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_SetFilePointerSync_004, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg = reinterpret_cast<napi_value>(0x1300);
    NVal mockNval = { env, nv };
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg));
    EXPECT_CALL(*libnMock, ToInt64()).WillOnce(Return(make_tuple(true, static_cast<int64_t>(-1))));
    EXPECT_CALL(*libnMock, CreateUndefined(_)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    auto res = RandomAccessFileNExporter::SetFilePointerSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(res, nullptr);
    EXPECT_EQ(rafEntity->filePointer, -1);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_SetFilePointerSync_005
 * @tc.desc: Test function of SetFilePointerSync for SUCCESS when fp == 0 (equal to zero).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_SetFilePointerSync_005, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg = reinterpret_cast<napi_value>(0x1300);
    NVal mockNval = { env, nv };
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg));
    EXPECT_CALL(*libnMock, ToInt64()).WillOnce(Return(make_tuple(true, static_cast<int64_t>(0))));
    EXPECT_CALL(*libnMock, CreateUndefined(_)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    auto res = RandomAccessFileNExporter::SetFilePointerSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(res, nullptr);
    EXPECT_EQ(rafEntity->filePointer, 0);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_SetFilePointerSync_006
 * @tc.desc: Test function of SetFilePointerSync for SUCCESS when fp > 0 (greater than zero).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_SetFilePointerSync_006, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg = reinterpret_cast<napi_value>(0x1300);
    NVal mockNval = { env, nv };
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg));
    EXPECT_CALL(*libnMock, ToInt64()).WillOnce(Return(make_tuple(true, static_cast<int64_t>(100))));
    EXPECT_CALL(*libnMock, CreateUndefined(_)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    auto res = RandomAccessFileNExporter::SetFilePointerSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(res, nullptr);
    EXPECT_EQ(rafEntity->filePointer, 100);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_ReadSync_001
 * @tc.desc: Test function of ReadSync for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_ReadSync_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::ReadSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_ReadSync_002
 * @tc.desc: Test function of ReadSync for FAILURE when GetRAFEntity fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_ReadSync_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::ReadSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_ReadSync_003
 * @tc.desc: Test function of ReadSync for FAILURE when GetReadArg fails (ToArraybuffer fails).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_ReadSync_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg1 = reinterpret_cast<napi_value>(0x1300);
    napi_value arg2 = reinterpret_cast<napi_value>(0x1400);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg1));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::SECOND)).WillOnce(Return(arg2));
    EXPECT_CALL(*libnMock, ToArraybuffer()).WillOnce(Return(make_tuple(false, nullptr, 0)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(2);
    auto res = RandomAccessFileNExporter::ReadSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_ReadSync_004
 * @tc.desc: Test function of ReadSync for FAILURE when DoReadRAF fails (actLen < 0).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_ReadSync_004, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg1 = reinterpret_cast<napi_value>(0x1300);
    napi_value arg2 = reinterpret_cast<napi_value>(0x1400);
    char buf[4] = {0};
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg1));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::SECOND)).WillOnce(Return(arg2));
    EXPECT_CALL(*libnMock, ToArraybuffer()).WillOnce(Return(make_tuple(true, buf, static_cast<size_t>(4))));
    EXPECT_CALL(*libnMock, HasProp("length")).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, HasProp("offset")).WillOnce(Return(false));
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(_)).Times(AnyNumber());
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::ReadSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    libnMock->VerifyAndClearErr(13900001, "Operation not permitted");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_ReadSync_005
 * @tc.desc: Test function of ReadSync for SUCCESS with offset < 0 (CalculateOffset true branch).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_ReadSync_005, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg1 = reinterpret_cast<napi_value>(0x1300);
    napi_value arg2 = reinterpret_cast<napi_value>(0x1400);
    NVal mockNval = { env, nv };
    char buf[4] = {0};
    rafEntity->filePointer = 10;
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg1));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::SECOND)).WillOnce(Return(arg2));
    EXPECT_CALL(*libnMock, ToArraybuffer()).WillOnce(Return(make_tuple(true, buf, static_cast<size_t>(4))));
    EXPECT_CALL(*libnMock, HasProp("length")).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, HasProp("offset")).WillOnce(Return(false));
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(4));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(_)).Times(AnyNumber());
    EXPECT_CALL(*libnMock, CreateInt64(_, _)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    auto res = RandomAccessFileNExporter::ReadSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_ReadSync_006
 * @tc.desc: Test function of ReadSync for SUCCESS with offset >= 0 (CalculateOffset false branch).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_ReadSync_006, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg1 = reinterpret_cast<napi_value>(0x1300);
    napi_value arg2 = reinterpret_cast<napi_value>(0x1400);
    NVal mockNval = { env, nv };
    char buf[4] = {0};
    rafEntity->filePointer = 10;
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg1));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::SECOND)).WillOnce(Return(arg2));
    EXPECT_CALL(*libnMock, ToArraybuffer()).WillOnce(Return(make_tuple(true, buf, static_cast<size_t>(4))));
    EXPECT_CALL(*libnMock, HasProp("length")).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, HasProp("offset")).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetProp("offset")).WillRepeatedly(Return(NVal(env, arg2)));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ToInt64()).WillOnce(Return(make_tuple(true, static_cast<int64_t>(0))));
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(4));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(_)).Times(AnyNumber());
    EXPECT_CALL(*libnMock, CreateInt64(_, _)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    auto res = RandomAccessFileNExporter::ReadSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_Read_001
 * @tc.desc: Test function of Read for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_Read_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::Read(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_Read_002
 * @tc.desc: Test function of Read for FAILURE when GetRAFEntity fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_Read_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::Read(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_Read_003
 * @tc.desc: Test function of Read for FAILURE when GetReadArg fails in ReadExec.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_Read_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg1 = reinterpret_cast<napi_value>(0x1300);
    napi_value arg2 = reinterpret_cast<napi_value>(0x1400);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg1));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::SECOND)).WillOnce(Return(arg2));
    EXPECT_CALL(*libnMock, ToArraybuffer()).WillOnce(Return(make_tuple(false, nullptr, 0)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(2);
    auto res = RandomAccessFileNExporter::Read(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_WriteSync_001
 * @tc.desc: Test function of WriteSync for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_WriteSync_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::WriteSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_WriteSync_002
 * @tc.desc: Test function of WriteSync for FAILURE when GetRAFEntity fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_WriteSync_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::WriteSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_WriteSync_003
 * @tc.desc: Test function of WriteSync for FAILURE when GetWriteArg fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_WriteSync_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg1 = reinterpret_cast<napi_value>(0x1300);
    napi_value arg2 = reinterpret_cast<napi_value>(0x1400);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg1));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::SECOND)).WillOnce(Return(arg2));
    EXPECT_CALL(*libnMock, GetProp("encoding")).WillOnce(Return(NVal(env, arg2)));
    EXPECT_CALL(*libnMock, TypeIs(napi_string)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ToArraybuffer()).WillOnce(Return(make_tuple(false, nullptr, 0)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(2);
    auto res = RandomAccessFileNExporter::WriteSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_WriteSync_004
 * @tc.desc: Test function of WriteSync for FAILURE when DoWriteRAF fails (writeLen < 0).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_WriteSync_004, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg1 = reinterpret_cast<napi_value>(0x1300);
    napi_value arg2 = reinterpret_cast<napi_value>(0x1400);
    char buf[4] = {'t', 'e', 's', 't'};
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg1));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::SECOND)).WillOnce(Return(arg2));
    EXPECT_CALL(*libnMock, GetProp("encoding")).WillOnce(Return(NVal(env, arg2)));
    EXPECT_CALL(*libnMock, TypeIs(napi_string)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ToArraybuffer()).WillOnce(Return(make_tuple(true, buf, static_cast<size_t>(4))));
    EXPECT_CALL(*libnMock, HasProp("length")).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, HasProp("offset")).WillOnce(Return(false));
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(_)).Times(AnyNumber());
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::WriteSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    libnMock->VerifyAndClearErr(13900001, "Operation not permitted");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_WriteSync_005
 * @tc.desc: Test function of WriteSync for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_WriteSync_005, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg1 = reinterpret_cast<napi_value>(0x1300);
    napi_value arg2 = reinterpret_cast<napi_value>(0x1400);
    NVal mockNval = { env, nv };
    char buf[4] = {'t', 'e', 's', 't'};
    rafEntity->filePointer = 0;
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg1));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::SECOND)).WillOnce(Return(arg2));
    EXPECT_CALL(*libnMock, GetProp("encoding")).WillOnce(Return(NVal(env, arg2)));
    EXPECT_CALL(*libnMock, TypeIs(napi_string)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ToArraybuffer()).WillOnce(Return(make_tuple(true, buf, static_cast<size_t>(4))));
    EXPECT_CALL(*libnMock, HasProp("length")).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, HasProp("offset")).WillOnce(Return(false));
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(4));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(_)).Times(AnyNumber());
    EXPECT_CALL(*libnMock, CreateInt64(_, _)).WillOnce(Return(mockNval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    auto res = RandomAccessFileNExporter::WriteSync(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_Write_001
 * @tc.desc: Test function of Write for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_Write_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::Write(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_Write_002
 * @tc.desc: Test function of Write for FAILURE when GetRAFEntity fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_Write_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::Write(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_Write_003
 * @tc.desc: Test function of Write for FAILURE when GetWriteArg fails in WriteExec.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_Write_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value arg1 = reinterpret_cast<napi_value>(0x1300);
    napi_value arg2 = reinterpret_cast<napi_value>(0x1400);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::FIRST)).WillOnce(Return(arg1));
    EXPECT_CALL(*libnMock, GetArg(NARG_POS::SECOND)).WillOnce(Return(arg2));
    EXPECT_CALL(*libnMock, GetProp("encoding")).WillOnce(Return(NVal(env, arg2)));
    EXPECT_CALL(*libnMock, TypeIs(napi_string)).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ToArraybuffer()).WillOnce(Return(make_tuple(false, nullptr, 0)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(2);
    auto res = RandomAccessFileNExporter::Write(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_Constructor_001
 * @tc.desc: Test function of Constructor for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_Constructor_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::Constructor(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_Constructor_002
 * @tc.desc: Test function of Constructor for FAILURE when SetEntityFor fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_Constructor_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_wrap(_, _, _, _, _, _)).WillOnce(Return(napi_invalid_arg));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::Constructor(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_Constructor_003
 * @tc.desc: Test function of Constructor for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_Constructor_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_wrap(_, _, _, _, _, _)).WillOnce(Return(napi_ok));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(0);
    auto res = RandomAccessFileNExporter::Constructor(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetReadStream_001
 * @tc.desc: Test function of GetReadStream for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetReadStream_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetReadStream(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetReadStream_002
 * @tc.desc: Test function of GetReadStream for FAILURE when GetRAFEntity fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetReadStream_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetReadStream(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetReadStream_003
 * @tc.desc: Test function of GetReadStream for FAILURE when permission check fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetReadStream_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    auto fcntlMock = FcntlMock::GetMock();
    FcntlMock::EnableMock();
    EXPECT_CALL(*fcntlMock, fcntl(rafEntity->fd.get()->GetFD(), F_GETFL)).WillOnce(Return(O_WRONLY));
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetReadStream(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fcntlMock.get());
    libnMock->VerifyAndClearErr(13900012, "Permission denied");
    FcntlMock::DisableMock();
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetWriteStream_001
 * @tc.desc: Test function of GetWriteStream for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetWriteStream_001, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetWriteStream(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900020, "Invalid argument");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetWriteStream_002
 * @tc.desc: Test function of GetWriteStream for FAILURE when GetRAFEntity fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetWriteStream_002, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetWriteStream(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    libnMock->VerifyAndClearErr(13900005, "I/O error");
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_GetWriteStream_003
 * @tc.desc: Test function of GetWriteStream for FAILURE when permission check fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_GetWriteStream_003, TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    auto libnMock = LibnMock::GetMock();
    auto fcntlMock = FcntlMock::GetMock();
    FcntlMock::EnableMock();
    EXPECT_CALL(*fcntlMock, fcntl(rafEntity->fd.get()->GetFD(), F_GETFL)).WillOnce(Return(O_RDONLY));
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);
    auto res = RandomAccessFileNExporter::GetWriteStream(env, mInfo);
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fcntlMock.get());
    libnMock->VerifyAndClearErr(13900012, "Permission denied");
    FcntlMock::DisableMock();
    EXPECT_EQ(res, nullptr);
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test