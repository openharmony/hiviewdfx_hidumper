/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef FD_OUTPUT_H
#define FD_OUTPUT_H

#include <string>
#include <vector>
#include <mutex>

#include "common.h"
#include "common/dumper_parameter.h"
#include "hidumper_executor.h"
#include "raw_param.h"

#define HIDUMPER_DEBUG
#ifdef HIDUMPER_DEBUG
#define LOG_DEBUG_DUMP(format, ...) \
    do { \
        fprintf(stdout, "Debug:" format "\n", ##__VA_ARGS__); \
    } while (0)
#else
#define LOG_DEBUG(format, ...)
#endif

namespace OHOS {
namespace HiviewDFX {
class FDOutput : public HidumperExecutor {
public:
    FDOutput();
    ~FDOutput();
    DumpStatus PreExecute(const std::shared_ptr<DumperParameter>& parameter,
        StringMatrix dumpDatas) override;
    DumpStatus Execute() override;
    DumpStatus AfterExecute() override;
    void OutMethod();
    void NewLineMethod(std::string &str);

private:
    int fd_;
    std::string path_;
    StringMatrix dumpDatas_;
    std::string dataStr_;
    std::vector<std::string> lineData_;
    std::shared_ptr<RawParam> ptrReqCtl_;
    static const mode_t OPEN_ARGV;
    std::mutex mutex_;
    void WriteToFd(std::string &str);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // FD_OUTPUT_H
