/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "dump_manager_client.h"
#include <iservice_registry.h>
#include <string_ex.h>
#include <unistd.h>
#include "common.h"
#include "hilog_wrapper.h"
#include "dump_errors.h"
#include "inner/dump_service_id.h"
#include "dump_on_demand_load.h"
namespace OHOS {
namespace HiviewDFX {
static constexpr int32_t SLEEP_DUR = 5 * 1000 * 1000;
static constexpr int32_t SLEEP_UNIT = 100 * 1000;

DumpManagerClient::DumpManagerClient()
{
}

DumpManagerClient::~DumpManagerClient()
{
}

int32_t DumpManagerClient::Request(std::vector<std::u16string> &args, int outfd)
{
    if ((args.size() < 1) || (outfd < 0)) {
        DUMPER_HILOGE(MODULE_CLIENT, "args or outfd failed.");
        return DumpStatus::DUMP_FAIL;
    }
    for (size_t i = 0; i < args.size(); i++) {
        std::string trimArg = TrimStr(Str16ToStr8(args[i]));
        if (strlen(trimArg.c_str()) < 1) {
            DUMPER_HILOGE(MODULE_CLIENT, "trimArg empty.");
            return DumpStatus::DUMP_FAIL;
        }
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        DUMPER_HILOGE(MODULE_CLIENT, "sam is nullptr.");
        return ERROR_GET_SYSTEM_ABILITY_MANAGER;
    }
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID);
    if (remoteObject == nullptr) {
        ErrCode retStart = OnDemandStart(sam, remoteObject);
        if (remoteObject == nullptr || retStart != ERR_OK) {
            DUMPER_HILOGE(MODULE_CLIENT, "remoteObject is nullptr.");
            return ERROR_GET_DUMPER_SERVICE;
        }
    }
    sptr<IDumpBroker> proxy = iface_cast<IDumpBroker>(remoteObject);
    if (proxy == nullptr) {
        DUMPER_HILOGE(MODULE_CLIENT, "proxy == nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    int32_t ret = proxy->Request(args, outfd);
    DUMPER_HILOGD(MODULE_CLIENT, "debug|ret = %{public}d", ret);
    return ret;
}

ErrCode DumpManagerClient::OnDemandStart(sptr<ISystemAbilityManager> sam, sptr<IRemoteObject> &remoteObject)
{
    sptr<OnDemandLoadCallback> loadCallback = new OnDemandLoadCallback();
    int32_t result = sam->LoadSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID, loadCallback);
    if (result != ERR_OK) {
        DUMPER_HILOGE(MODULE_CLIENT, "systemAbilityId:%{public}d load failed, result code:%{public}d",
            DFX_SYS_HIDUMPER_ABILITY_ID, result);
        return ERROR_GET_DUMPER_SERVICE;
    }

    int32_t loop = SLEEP_DUR / SLEEP_UNIT;
    while (loop-- > 0) {
        if (!loadCallback->CheckLoadSystemAbilityStatus()) {
            usleep(SLEEP_UNIT);
            continue;
        }
        remoteObject = sam->CheckSystemAbility(DFX_SYS_HIDUMPER_ABILITY_ID);
        if (remoteObject != nullptr) {
            return ERR_OK;
        } else {
            usleep(SLEEP_UNIT);
        }
    }

    DUMPER_HILOGD(MODULE_CLIENT, "debug|on demand start fail");
    return ERROR_GET_DUMPER_SERVICE;
}
} // namespace HiviewDFX
} // namespace OHOS
