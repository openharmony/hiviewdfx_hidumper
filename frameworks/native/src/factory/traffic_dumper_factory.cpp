/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "factory/traffic_dumper_factory.h"
#include "executor/traffic_dumper.h"

namespace OHOS {
namespace HiviewDFX {
std::shared_ptr<HidumperExecutor> TrafficDumperFactory::CreateExecutor()
{
    return std::make_shared<TrafficDumper>();
}
}  // namespace HiviewDFX
}  // namespace OHOS
