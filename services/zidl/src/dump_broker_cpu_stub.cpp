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
#include "dump_broker_cpu_stub.h"
#include <message_parcel.h>
#include "dump_errors.h"
#include "hilog_wrapper.h"
#include "dump_cpu_data.h"
namespace OHOS {
namespace HiviewDFX {
int DumpBrokerCpuStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descripter = DumpBrokerCpuStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        return ERROR_GET_DUMPER_SERVICE;
    }
    int ret = ERR_OK;
    switch (code) {
        case static_cast<int>(IDumpCpuBroker::DUMP_REQUEST_CPUINFO): {
            std::shared_ptr<DumpCpuData> dumpCpuData(data.ReadParcelable<DumpCpuData>());
            if (dumpCpuData == nullptr) {
                DUMPER_HILOGE(MODULE_CPU_ZIDL, "error|DumpBrokerCpuStub error");
                return ERROR_READ_PARCEL;
            }
            int32_t res = Request(*dumpCpuData);
            DumpCpuData dump = *dumpCpuData;
            if (!reply.WriteParcelable(&dump)) {
                return ERROR_WRITE_PARCEL;
            }
            if (!reply.WriteInt32(res)) {
                return ERROR_WRITE_PARCEL;
            }
            break;
        }
        default: {
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS