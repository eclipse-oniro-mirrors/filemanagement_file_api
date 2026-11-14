/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_DATASHARE_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_DATASHARE_MOCK_H

#include "datashare_helper.h"

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace OHOS::DataShare;

// Mocks DataShareHelper for tests that must not depend on a live datashare service (e.g. medialibrary).
// The static DataShareHelper::Creator is intercepted by a link-time override defined in datashare_mock.cpp,
// which forwards to DataShareMock::Creator below; OpenFile is mocked through the virtual interface.
class DataShareMock final : public DataShareHelper {
public:
    MOCK_METHOD(int, OpenFile, (Uri &uri, const std::string &mode), (override));
    // Stands in for the static DataShareHelper::Creator used by OpenFileByDatashare
    MOCK_METHOD(std::shared_ptr<DataShareHelper>, Creator,
        (const OHOS::sptr<OHOS::IRemoteObject> &token, const std::string &strUri), ());

    bool Release() override
    {
        return true;
    }
    std::vector<std::string> GetFileTypes(Uri &uri, const std::string &mimeTypeFilter) override
    {
        return {};
    }
    int OpenFileWithErrCode(Uri &uri, const std::string &mode, int32_t &errCode) override
    {
        errCode = 0;
        return -1;
    }
    int OpenRawFile(Uri &uri, const std::string &mode) override
    {
        return -1;
    }
    int Insert(Uri &uri, const DataShareValuesBucket &value) override
    {
        return -1;
    }
    int InsertExt(Uri &uri, const DataShareValuesBucket &value, std::string &result) override
    {
        return -1;
    }
    int Update(Uri &uri, const DataSharePredicates &predicates, const DataShareValuesBucket &value) override
    {
        return -1;
    }
    int BatchUpdate(const UpdateOperations &operations, std::vector<BatchUpdateResult> &results) override
    {
        return -1;
    }
    int Delete(Uri &uri, const DataSharePredicates &predicates) override
    {
        return -1;
    }
    std::shared_ptr<DataShareResultSet> Query(Uri &uri, const DataSharePredicates &predicates,
        std::vector<std::string> &columns, DatashareBusinessError *businessError) override
    {
        return nullptr;
    }
    std::string GetType(Uri &uri) override
    {
        return "";
    }
    int BatchInsert(Uri &uri, const std::vector<DataShareValuesBucket> &values) override
    {
        return -1;
    }
    int ExecuteBatch(const std::vector<OperationStatement> &statements, ExecResultSet &result) override
    {
        return -1;
    }
    int RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override
    {
        return -1;
    }
    int UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override
    {
        return -1;
    }
    void NotifyChange(const Uri &uri) override {}
    int RegisterObserverExtProvider(const Uri &uri, std::shared_ptr<DataShareObserver> dataObserver,
        bool isDescendants) override
    {
        return -1;
    }
    int UnregisterObserverExtProvider(const Uri &uri, std::shared_ptr<DataShareObserver> dataObserver) override
    {
        return -1;
    }
    void NotifyChangeExtProvider(const DataShareObserver::ChangeInfo &changeInfo) override {}
    Uri NormalizeUri(Uri &uri) override
    {
        return uri;
    }
    Uri DenormalizeUri(Uri &uri) override
    {
        return uri;
    }
    int AddQueryTemplate(const std::string &uri, int64_t subscriberId, Template &tpl) override
    {
        return -1;
    }
    int DelQueryTemplate(const std::string &uri, int64_t subscriberId) override
    {
        return -1;
    }
    std::vector<OperationResult> Publish(const Data &data, const std::string &bundleName) override
    {
        return {};
    }
    Data GetPublishedData(const std::string &bundleName, int &resultCode) override
    {
        resultCode = -1;
        return Data();
    }
    std::vector<OperationResult> SubscribeRdbData(const std::vector<std::string> &uris,
        const TemplateId &templateId, const std::function<void(const RdbChangeNode &changeNode)> &callback)
        override
    {
        return {};
    }
    std::vector<OperationResult> UnsubscribeRdbData(const std::vector<std::string> &uris,
        const TemplateId &templateId) override
    {
        return {};
    }
    std::vector<OperationResult> EnableRdbSubs(const std::vector<std::string> &uris,
        const TemplateId &templateId) override
    {
        return {};
    }
    std::vector<OperationResult> DisableRdbSubs(const std::vector<std::string> &uris,
        const TemplateId &templateId) override
    {
        return {};
    }
    std::vector<OperationResult> SubscribePublishedData(const std::vector<std::string> &uris,
        int64_t subscriberId, const std::function<void(const PublishedDataChangeNode &changeNode)> &callback)
        override
    {
        return {};
    }
    std::vector<OperationResult> UnsubscribePublishedData(const std::vector<std::string> &uris,
        int64_t subscriberId) override
    {
        return {};
    }
    std::vector<OperationResult> EnablePubSubs(const std::vector<std::string> &uris, int64_t subscriberId)
        override
    {
        return {};
    }
    std::vector<OperationResult> DisablePubSubs(const std::vector<std::string> &uris, int64_t subscriberId)
        override
    {
        return {};
    }

    DataShareMock() = default;
    ~DataShareMock() = default;

public:
    static std::shared_ptr<DataShareMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<DataShareMock> dataShareMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_DATASHARE_MOCK_H
