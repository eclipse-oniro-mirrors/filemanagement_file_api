/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <cerrno>
#include <string>
#include <vector>
#include "liburing.h"
#include "hyperaio.h"

namespace OHOS::HyperAio {
    using namespace std;
    using namespace testing;

    class HyperAioTest : public testing::Test {
    public:
        static void SetUpTestCase(void) {};
        static void TearDownTestCase() {};
        void SetUp() {};
        void TearDown() {};
    };
#ifdef HYPERAIO_USE_LIBURING
    const uint64_t userData = 12345;
    const uint32_t len = 1024;
    const uint32_t batchSize = 300;
    HyperAio::ProcessIoResultCallBack callBack = [](std::unique_ptr<IoResponse> response) {
        GTEST_LOG_(INFO) << "HyperAioTest callBack";
    };
    /**
     * @tc.name: HyperAio_SupportIouring_0000
     * @tc.desc: Test function of SupportIouring() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_SupportIouring_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_SupportIouring_0000";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & IOURING_APP_PERMISSION) == 0, true);
        if ((result & IOURING_APP_PERMISSION) == 0) {
            return;
        }
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_SupportIouring_0000";
    }
    /**
     * @tc.name: HyperAio_CtxInit_0000
     * @tc.desc: Test function of CtxInit() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_CtxInit_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_CtxInit_0000";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        result = hyperAio_->CtxInit(&callBack);
        hyperAio_->stopThread_.store(true);
        EXPECT_EQ(result, 0);
        hyperAio_->DestroyCtx();
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_CtxInit_0000";
    }
    /**
     * @tc.name: HyperAio_CtxInit_0001
     * @tc.desc: Test function of CtxInit() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_CtxInit_0001, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_CtxInit_0001";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(nullptr);
        EXPECT_EQ(result, -EINVAL);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_CtxInit_0001";
    }
    /**
     * @tc.name: HyperAio_CtxInit_0002
     * @tc.desc: Test function of CtxInit() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_CtxInit_0002, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_CtxInit_0002";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        hyperAio_->DestroyCtx();
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_CtxInit_0002";
    }
    /**
     * @tc.name: HyperAio_CtxInit_0003
     * @tc.desc: Test function of CtxInit() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_CtxInit_0003, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_CtxInit_0003";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(nullptr);
        EXPECT_EQ(result, -EINVAL);
        result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        hyperAio_->DestroyCtx();
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_CtxInit_0003";
    }
    /**
     * @tc.name: HyperAio_StartOpenReqs_0000
     * @tc.desc: Test function of StartOpenReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartOpenReqs_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartOpenReqs_0000";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        result = hyperAio_->StartOpenReqs(nullptr);
        EXPECT_EQ(result, -EINVAL);
        hyperAio_->DestroyCtx();
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartOpenReqs_0000";
    }

    /**
     * @tc.name: HyperAio_StartOpenReqs_0001
     * @tc.desc: Test function of StartOpenReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartOpenReqs_0001, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartOpenReqs_0001";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        hyperAio_->initialized_.store(false);
        OpenInfo openInfo = {0, O_RDWR, 0, nullptr, userData};
        OpenReqs openReqs = {1, &openInfo};
        result = hyperAio_->StartOpenReqs(&openReqs);
        EXPECT_EQ(result, -EPERM);
        hyperAio_->initialized_.store(true);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartOpenReqs_0001";
    }

    /**
     * @tc.name: HyperAio_StartOpenReqs_0002
     * @tc.desc: Test function of StartOpenReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartOpenReqs_0002, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartOpenReqs_0002";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        OpenInfo openInfo = {0, O_RDWR, 0, nullptr, userData};
        OpenReqs openReqs = {1, &openInfo};
        result = hyperAio_->StartOpenReqs(&openReqs);
        EXPECT_EQ(result, 0);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartOpenReqs_0002";
    }

    /**
     * @tc.name: HyperAio_StartOpenReqs_0003
     * @tc.desc: Test function of StartOpenReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartOpenReqs_0003, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartOpenReqs_0003";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        auto openInfos = std::make_unique<OpenInfo[]>(batchSize);
        for (int i = 0; i < batchSize; ++i) {
            openInfos[i].dfd = 0;
            openInfos[i].flags = O_RDWR;
            openInfos[i].mode = 0;
            openInfos[i].path = nullptr;
            openInfos[i].userData = userData + i;
        }
        OpenReqs openReqs = {batchSize, openInfos.get()};
        result = hyperAio_->StartOpenReqs(&openReqs);
        EXPECT_EQ(result, 0);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartOpenReqs_0003";
    }

    /**
     * @tc.name: HyperAio_StartReadReqs_0000
     * @tc.desc: Test function of StartReadReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartReadReqs_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartReadReqs_0000";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        result = hyperAio_->StartReadReqs(nullptr);
        EXPECT_EQ(result, -EINVAL);
        result = hyperAio_->DestroyCtx();
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartReadReqs_0000";
    }

    /**
     * @tc.name: HyperAio_StartReadReqs_0001
     * @tc.desc: Test function of StartReadReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartReadReqs_0001, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartReadReqs_0001";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        hyperAio_->initialized_.store(false);
        ReadInfo readInfo = {0, len, 0, nullptr, userData};
        ReadReqs readReqs = {1, &readInfo};
        result = hyperAio_->StartReadReqs(&readReqs);
        EXPECT_EQ(result, -EPERM);
        hyperAio_->initialized_.store(true);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartReadReqs_0001";
    }

    /**
     * @tc.name: HyperAio_StartReadReqs_0002
     * @tc.desc: Test function of StartReadReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartReadReqs_0002, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartReadReqs_0002";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        ReadInfo readInfo = {0, len, 0, nullptr, userData};
        ReadReqs readReqs = {1, &readInfo};
        result = hyperAio_->StartReadReqs(&readReqs);
        EXPECT_EQ(result, 0);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartReadReqs_0002";
    }

    /**
     * @tc.name: HyperAio_StartCancelReqs_0000
     * @tc.desc: Test function of StartCancelReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartCancelReqs_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartCancelReqs_0000";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        result = hyperAio_->StartCancelReqs(nullptr);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartCancelReqs_0000";
    }

    /**
     * @tc.name: HyperAio_StartCancelReqs_0001
     * @tc.desc: Test function of StartCancelReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartCancelReqs_0001, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartCancelReqs_0001";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        hyperAio_->initialized_.store(false);
        CancelInfo cancelInfo = {userData, 0};
        CancelReqs cancelReqs = {1, &cancelInfo};
        result = hyperAio_->StartCancelReqs(&cancelReqs);
        EXPECT_EQ(result, -EPERM);
        hyperAio_->initialized_.store(true);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartCancelReqs_0001";
    }

    /**
     * @tc.name: HyperAio_StartCancelReqs_0002
     * @tc.desc: Test function of StartCancelReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartCancelReqs_0002, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartCancelReqs_0002";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->CtxInit(&callBack);
        CancelInfo cancelInfo = {userData, 0};
        CancelReqs cancelReqs = {1, &cancelInfo};
        result = hyperAio_->StartCancelReqs(&cancelReqs);
        EXPECT_EQ(result, 0);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartCancelReqs_0002";
    }
#endif
}
