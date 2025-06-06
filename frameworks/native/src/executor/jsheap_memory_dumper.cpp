/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "executor/jsheap_memory_dumper.h"
#include "dump_common_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
JsHeapMemoryDumper::JsHeapMemoryDumper()
{
    if (jsHeapInfo_ == nullptr) {
        jsHeapInfo_ = std::make_unique<DumpJsHeapInfo>();
    }
}

JsHeapMemoryDumper::~JsHeapMemoryDumper()
{
}

DumpStatus JsHeapMemoryDumper::PreExecute(const shared_ptr<DumperParameter> &parameter, StringMatrix dumpDatas)
{
    if (parameter == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "parameter is nullptr");
        return DumpStatus::DUMP_FAIL;
    }
    needGc_ = true;
    needSnapshot_ = true;
    needLeakobj_ = parameter->GetOpts().isDumpJsHeapLeakobj_;
    /* In the arkts runtime, DumpFormat::BINARY is used to generate rawheap files. */
    needBinary_ = parameter->GetOpts().dumpJsRawHeap_;
    if (parameter->GetOpts().isDumpJsHeapMemGC_) {
        needSnapshot_ = false;
        needLeakobj_ = false;
        needBinary_ = false;
        DUMPER_HILOGD(MODULE_SERVICE, "trigger gc, set needSnapshot_ false, needLeakobj_ false");
    }
    pid_ = static_cast<uint32_t>(parameter->GetOpts().dumpJsHeapMemPid_);
    tid_ = static_cast<uint32_t>(parameter->GetOpts().threadId_);
    dumpDatas_ = dumpDatas;
    return DumpStatus::DUMP_OK;
}

DumpStatus JsHeapMemoryDumper::Execute()
{
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = pid_;
    info.tid = tid_;
    info.needGc = needGc_;
    info.needSnapshot = needSnapshot_;
    info.needLeakobj = needLeakobj_;
    info.needBinary = needBinary_;

    if (dumpDatas_ != nullptr && jsHeapInfo_ != nullptr) {
        bool ret = jsHeapInfo_->DumpJsHeapMemory(info);
        ret ? status_ = DumpStatus::DUMP_OK : status_ = DumpStatus::DUMP_FAIL;
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "JsHeapMemoryDumper Execute nullptr");
        status_ = DumpStatus::DUMP_FAIL;
    }
    return status_;
}

DumpStatus JsHeapMemoryDumper::AfterExecute()
{
    return status_;
}
} // namespace HiviewDFX
} // namespace OHOS