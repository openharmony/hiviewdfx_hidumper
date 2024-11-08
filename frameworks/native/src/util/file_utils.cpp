/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <unistd.h>
#include <sys/stat.h>
#include "hilog_wrapper.h"
#include "util/file_utils.h"
using namespace std;
namespace OHOS {
namespace HiviewDFX {
static const std::string UNKNOWN = "unknown";
FileUtils::FileUtils()
{
}
FileUtils::~FileUtils()
{
}

bool FileUtils::CreateFolder(const string &path)
{
    if (!access(path.c_str(), F_OK) || path == "") {
        return true;
    }

    size_t pos = path.rfind("/");
    if (pos == string::npos) {
        return false;
    }

    string upperPath = path.substr(0, pos);
    if (CreateFolder(upperPath)) {
        if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) {
            if (errno != EEXIST) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool FileUtils::LoadStringFromProcCb(const std::string& path, bool oneLine, bool endWithoutN, const DataHandler& func)
{
    char canonicalPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), canonicalPath) == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "realpath failed, errno=%{public}d, path=%{public}s", errno, path.c_str());
        return false;
    }
    auto fp = std::unique_ptr<FILE, decltype(&fclose)>{fopen(canonicalPath, "re"), fclose};
    if (fp == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "fopen failed, errno=%{public}d, path=%{public}s", errno, path.c_str());
        return false;
    }
    char *lineBuf = nullptr;
    ssize_t lineLen;
    size_t lineAlloc = 0;
    while ((lineLen = getline(&lineBuf, &lineAlloc, fp.get())) > 0) {
        lineBuf[lineLen] = '\0';
        if (endWithoutN && lineBuf[lineLen-1] == '\n') {
            lineBuf[lineLen-1] = '\0';
        }
        const string content = lineBuf;
        func(content);
        if (oneLine) {
            break;
        }
    }
    if (lineBuf != nullptr) {
        free(lineBuf);
        lineBuf = nullptr;
    }
    return true;
}

string FileUtils::GetProcValue(const int32_t &pid, const string& path, const string& key)
{
    if (!DumpUtils::PathIsValid(path)) {
        DUMPER_HILOGE(MODULE_COMMON, "path is valid");
        return UNKNOWN;
    }
    auto fp = std::unique_ptr<FILE, decltype(&fclose)>{fopen(path.c_str(), "rd"), fclose};
    if (fp == nullptr) {
        DUMPER_HILOGE(MODULE_COMMON, "fopen failed");
        return UNKNOWN;
    }
    char *lineBuf = nullptr;
    ssize_t lineLen;
    size_t lineAlloc = 0;
    string content;
    while ((lineLen = getline(&lineBuf, &lineAlloc, fp.get())) > 0) {
        lineBuf[lineLen] = '\0';
        if (lineBuf[lineLen - 1] == '\n') {
            lineBuf[lineLen - 1] = '\0';
        }
        content = lineBuf;
        if (content.find(key) != std::string::npos) {
            break;
        }
        content = "";
    }
    if (lineBuf != nullptr) {
        free(lineBuf);
        lineBuf = nullptr;
    }
    if (!content.empty()) {
        vector<string> values;
        StringUtils::GetInstance().StringSplit(content, ":", values);
        if (values.empty()) {
            DUMPER_HILOGE(MODULE_SERVICE, "values is empty");
            return UNKNOWN;
        } else {
            return values[1].substr(1);
        }
    } else {
        DUMPER_HILOGE(MODULE_SERVICE, "content is empty");
        return UNKNOWN;
    }
}
} // namespace HiviewDFX
} // namespace OHOS