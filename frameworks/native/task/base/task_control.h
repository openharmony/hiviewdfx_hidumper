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

#ifndef HIVIEWDFX_HIDUMPER_TASK_CONTROL_H
#define HIVIEWDFX_HIDUMPER_TASK_CONTROL_H

#include <functional>
#include <memory>
#include <vector>
#include <map>

#include "data_inventory.h"
#include "dump_context.h"
#include "task/base/task_register.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {

struct TaskStatistcs {
    std::string taskName;
    std::vector<std::string> dependTaskNames;
    DumpStatus dumpStatus;
    uint64_t startTime;
    uint64_t duration;
    bool mandatory;
};

class TaskControl : public DelayedRefSingleton<TaskControl> {
    DECLARE_DELAYED_REF_SINGLETON(TaskControl)
public:
    DISALLOW_COPY_AND_MOVE(TaskControl);

    DumpStatus ExcuteTask(DataInventory& dataInventory,
                          const std::vector<TaskId>& taskIds, const std::shared_ptr<DumpContext>& dumpContext);
private:
    DumpStatus ExcuteTaskInner(DataInventory& dataInventory,
                               TaskCollection& tasks, const std::shared_ptr<DumpContext>& dumpContext);
    bool VerifyTaskTopo(const TaskCollection& taskTopo);
    void BuildTaskTopo(TaskId rootTaskId, TaskCollection& taskTopo);
    TaskCollection SelectRunnableTasks(TaskCollection& tasks);
    void ReleaseNoUsedData(DataInventory& dataInventory, const TaskCollection& tasks);
    void SubmitRunnableTasks(TaskCollection& tasks, DataInventory& dataInventory,
                             const std::shared_ptr<DumpContext>& dumpContext, std::vector<TaskStatistcs>& stat);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEWDFX_HIDUMPER_TASK_CONTROL_H
