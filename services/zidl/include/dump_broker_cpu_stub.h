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
#ifndef HIDUMPER_ZIDL_DUMP_BROKER_STUB_H
#define HIDUMPER_ZIDL_DUMP_BROKER_STUB_H
#include <iremote_stub.h>
#include <nocopyable.h>
#include "idump_cpu_broker.h"
namespace OHOS {
namespace HiviewDFX {
class DumpBrokerCpuStub : public IRemoteStub<IDumpCpuBroker> {
public:
    DISALLOW_COPY_AND_MOVE(DumpBrokerCpuStub);
    DumpBrokerCpuStub() = default;
    virtual ~DumpBrokerCpuStub() = default;
public:
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    int DumpCpuUsageOnly(MessageParcel& data, MessageParcel& reply);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_ZIDL_DUMP_BROKER_STUB_H
