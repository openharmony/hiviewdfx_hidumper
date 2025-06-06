/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "dump_common_utils.h"
#include <fcntl.h>
#include <file_ex.h>
#include <securec.h>
#include <string_ex.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include "hilog_wrapper.h"
#ifdef HIDUMPER_HIVIEWDFX_HISYSEVENT_ENABLE
#include "hisysevent.h"
#endif
#include "sys/stat.h"
#include "util/string_utils.h"
#include "util/file_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int LINE_ITEM_MIN = 2;
constexpr int LINE_KEY = 0;
constexpr int LINE_VALUE = 1;
constexpr int LINE_VALUE_0 = 0;
constexpr int UNSET = -1;
constexpr double ONE_DAY_TO_SECONDS = 24 * 60 * 60;
static const std::string CPU_STR = "cpu";
static const std::string STORAGE_PATH_PREFIX = "/data/storage/el";
static const size_t STORAGE_PATH_SIZE = STORAGE_PATH_PREFIX.size();
static const int TM_START_YEAR = 1900;
static const int DEC_SYSTEM_VALUE = 10;
}

std::vector<std::string> DumpCommonUtils::GetSubNodes(const std::string &path, bool digit)
{
    std::vector<std::string> subNodes;
    auto dir = opendir(path.c_str());
    if (dir == nullptr) {
        return subNodes;
    }
    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        std::string childNode = ent->d_name;
        if (childNode == "." || childNode == "..") {
            continue;
        }
        if (digit && !IsNumericStr(childNode)) {
            continue;
        }
        subNodes.push_back(childNode);
    }
    closedir(dir);
    return subNodes;
}

// the parameter of path should be full.
bool DumpCommonUtils::IsDirectory(const std::string &path)
{
    struct stat statBuffer;
    if (stat(path.c_str(), &statBuffer) == 0 && S_ISDIR(statBuffer.st_mode)) {
        return true;
    }
    return false;
}

std::vector<std::string> DumpCommonUtils::GetSubDir(const std::string &path, bool digit)
{
    std::vector<std::string> subDirs;
    auto dir = opendir(path.c_str());
    if (dir == nullptr) {
        DUMPER_HILOGE(MODULE_SERVICE, "failed to open dir: %{public}s, errno: %{public}d", path.c_str(), errno);
        return subDirs;
    }
    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        std::string childNode = ent->d_name;
        if (childNode == "." || childNode == "..") {
            continue;
        }
        if (digit && !IsNumericStr(childNode)) {
            continue;
        }
        if (!IsDirectory(path + "/" + childNode)) {
            continue; // skip directory
        }
        subDirs.push_back(childNode);
    }
    closedir(dir);
    return subDirs;
}

std::vector<int32_t> DumpCommonUtils::GetAllPids()
{
    std::string path = "/proc";
    std::vector<int32_t> pids;
    std::vector<std::string> allPids = GetSubDir(path, true);
    for (const auto &pid : allPids) {
        if (!IsNumericStr(pid)) {
            continue;
        }
        pids.push_back(std::stoi(pid));
    }
    return pids;
}

DumpCommonUtils::CpuInfo::CpuInfo()
{
    id_ = UNSET;
}

bool DumpCommonUtils::GetCpuInfos(std::vector<CpuInfo> &infos)
{
    std::vector<std::string> names;
    if (!GetNamesInFolder("/sys/devices/system/cpu/", names)) {
        return false;
    }
    for (size_t i = 0; i < names.size(); i++) {
        std::string name = names[i];
        if (!StartWith(name, CPU_STR)) {
            continue;
        }
        std::string cpuId = name.substr(CPU_STR.size());
        if (cpuId.empty() || (!IsNumericStr(cpuId))) {
            continue;
        }
        CpuInfo cpuInfo;
        StrToInt(cpuId, cpuInfo.id_);
        infos.push_back(cpuInfo);
    }
    return true;
}

DumpCommonUtils::PidInfo::PidInfo()
{
    Reset();
}

void DumpCommonUtils::PidInfo::Reset()
{
    pid_ = UNSET;
    uid_ = UNSET;
    gid_ = UNSET;
    ppid_ = UNSET;
    name_.clear();
    cmdline_.clear();
}

bool DumpCommonUtils::GetPidInfos(std::vector<PidInfo> &infos, bool all)
{
    std::vector<std::string> names;
    if (!GetNamesInFolder("/proc/", names)) {
        return false;
    }
    for (size_t i = 0; i < names.size(); i++) {
        std::string name = names[i];
        if (name.empty()) {
            continue;
        }
        if (!IsNumericStr(name)) {
            continue;
        }
        PidInfo pidInfo;
        StrToInt(name, pidInfo.pid_);
        GetProcessInfo(pidInfo.pid_, pidInfo);
        if (all) {
            GetProcessNameByPid(pidInfo.pid_, pidInfo.cmdline_);
        }
        infos.push_back(pidInfo);
    }
    return true;
}

bool DumpCommonUtils::IsUserPid(const std::string &pid)
{
    string path = "/proc/" + pid + "/smaps";
    string lineContent;
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(path, true, false, [&](const string& line) -> void {
        lineContent += line;
    });
    if (!ret) {
        return false;
    }
    if (!lineContent.empty()) {
        return true;
    }
    return false;
}

bool DumpCommonUtils::GetUserPids(std::vector<int> &pids)
{
    std::vector<std::string> files;
    if (!GetNamesInFolder("/proc/", files)) {
        return false;
    }

    for (auto file : files) {
        if (file.empty()) {
            continue;
        }
        if (!IsNumericStr(file)) {
            continue;
        }

        if (!IsUserPid(file)) {
            continue;
        }

        int pid;
        StrToInt(file, pid);

        pids.push_back(pid);
    }
    return true;
}

bool DumpCommonUtils::GetLinesInFile(const std::string& file, std::vector<std::string>& lines)
{
    std::string content;
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(file, false, false, [&](const std::string& line) -> void {
        content += line;
    });
    if (!ret) {
        return false;
    }
    SplitStr(content, "\n", lines);
    return true;
}

bool DumpCommonUtils::GetNamesInFolder(const std::string& folder, std::vector<std::string>& names)
{
    bool ret = false;
    DIR* dir = nullptr;
    if ((dir = opendir(folder.c_str())) != nullptr) {
        dirent* ptr = nullptr;
        while ((ptr = readdir(dir)) != nullptr) {
            std::string name = ptr->d_name;
            if ((name == ".") || (name == "..")) {
                continue;
            }
            names.push_back(name);
        }
        closedir(dir);
        ret = true;
    }
    return ret;
}

bool DumpCommonUtils::StartWith(const std::string& str, const std::string& head)
{
    if (str.length() < head.length()) {
        return false;
    }
    return (str.compare(0, head.size(), head) == 0);
}

bool DumpCommonUtils::GetProcessNameByPid(int pid, std::string& name)
{
    char filesysdir[128] = { 0 };
    if (sprintf_s(filesysdir, sizeof(filesysdir), "/proc/%d/cmdline", pid) < 0) {
        return false;
    }
    std::string filePath = filesysdir;
    std::string content;
    bool ret = FileUtils::GetInstance().LoadStringFromProcCb(filePath, false, false, [&](const string& line) -> void {
        content += line;
    });
    if (!ret) {
        return false;
    }
    vector<string> names;
    StringUtils::GetInstance().StringSplit(content, " ", names);
    if (names.empty()) {
        return false;
    }
    vector<string> longNames;
    StringUtils::GetInstance().StringSplit(names[0], "/", longNames);
    if (longNames.empty()) {
        return false;
    }
    if (names[0].find("/bin") != std::string::npos) {
        name = longNames[longNames.size() - 1];
    } else {
        name = names[0];
    }
    return true;
}

bool DumpCommonUtils::GetProcessInfo(int pid, PidInfo &info)
{
    info.Reset();
    char filesysdir[128] = { 0 };
    if (sprintf_s(filesysdir, sizeof(filesysdir), "/proc/%d/status", pid) < 0) {
        return false;
    }
    std::string file = filesysdir;
    std::vector<std::string> lines;
    if (!GetLinesInFile(file, lines)) {
        return false;
    }
    const std::string splitKeyValueToken = ":";
    const std::string splitValuesToken = "\t";
    for (size_t i = 0; i < lines.size(); i++) {
        std::vector<std::string> keyValue;
        SplitStr(lines[i], splitKeyValueToken, keyValue);
        if (keyValue.size() < LINE_ITEM_MIN) {
            continue;
        }
        std::string key = keyValue[LINE_KEY];
        std::string val = TrimStr(keyValue[LINE_VALUE], '\t');
        std::vector<std::string> values;
        SplitStr(val, splitValuesToken, values, true);
        if (values.empty()) {
            continue;
        }
        std::string val0 = values[LINE_VALUE_0];
        if (key == "Pid") {
            StrToInt(val0, info.pid_);
        } else if (key == "PPid") {
            StrToInt(val0, info.ppid_);
        } else if (key == "Uid") {
            StrToInt(val0, info.uid_);
        } else if (key == "Gid") {
            StrToInt(val0, info.gid_);
        } else if (key == "Name") {
            info.name_ = val;
        } else {
            continue;
        }
        if ((info.pid_ > UNSET) && (info.ppid_ > UNSET) && (info.uid_ > UNSET) && (info.gid_ > UNSET)) {
            return true;
        }
    }
    return false;
}

int DumpCommonUtils::FindStorageDirSecondDigitIndex(const std::string& fullFileName)
{
    size_t fileNameSize = fullFileName.size();
    if (STORAGE_PATH_SIZE >= fileNameSize) {
        return static_cast<int>(fileNameSize);
    }
    if (fullFileName.compare(0, STORAGE_PATH_SIZE, STORAGE_PATH_PREFIX) != 0 ||
        !std::isdigit(fullFileName[STORAGE_PATH_SIZE])) {
            return static_cast<int>(fileNameSize);
    }
    for (size_t i = STORAGE_PATH_SIZE + 1; i < fileNameSize; i++) {
        if (std::isdigit(fullFileName[i])) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(fileNameSize);
}

void DumpCommonUtils::ReportCmdUsage(const std::unique_ptr<DumperSysEventParams>& param)
{
    ClearHisyseventTmpFile();
    int fd = -1;
    fd = TEMP_FAILURE_RETRY(open(HISYSEVENT_TMP_FILE.c_str(), O_RDWR | O_CREAT | O_APPEND,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (fd < 0) {
        DUMPER_HILOGE(MODULE_COMMON, "open hisysevent_tmp file error: %{public}d", errno);
        return;
    }
    std::string content = "";
    if (!OHOS::LoadStringFromFd(fd, content)) {
        DUMPER_HILOGE(MODULE_COMMON, "LoadStringFromFd error! %{public}d", errno);
        close(fd);
        return;
    }
    std::string option = "";
    param->errorCode == 0 ? option = "OPT:" + param->opt + " SUB_OPT:" + param->subOpt + "\n" :
        option = "ERROR_MESSAGE:" + param->errorMsg + "\n";
    if (content.find(option) != std::string::npos) {
        DUMPER_HILOGD(MODULE_COMMON, "hisysevent data contain option, not report");
        close(fd);
        return;
    }
    std::string callerProcess = "unknown";
    DumpCommonUtils::GetProcessNameByPid(param->callerPpid, callerProcess);
#ifdef HIDUMPER_HIVIEWDFX_HISYSEVENT_ENABLE
    int ret = HiSysEventWrite(HiSysEvent::Domain::HIDUMPER, "CMD_USAGE",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "OPT", param->opt, "CALLER", callerProcess, "SUB_OPT", param->subOpt, "TARGET", param->target,
        "ARGS", param->arguments, "ERROR_CODE", param->errorCode, "ERROR_MESSAGE", param->errorMsg);
    if (ret != 0) {
        DUMPER_HILOGE(MODULE_COMMON, "hisysevent report hidumper usage failed! ret %{public}d.", ret);
        close(fd);
        return;
    }
#endif
    SaveStringToFd(fd, option.c_str());
    close(fd);
}

void DumpCommonUtils::ClearHisyseventTmpFile()
{
    if (!OHOS::FileExists(HISYSEVENT_TMP_FILE)) {
        DUMPER_HILOGE(MODULE_COMMON, "HISYSEVENT_TMP_FILE not exists");
        return;
    }
    time_t lastWriteTime;
    if (!FileUtils::GetInstance().GetLastWriteTime(HISYSEVENT_TMP_FILE, lastWriteTime)) {
        DUMPER_HILOGE(MODULE_COMMON, "GetLastWriteTime failed");
        return;
    }
    time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm currentTimeData = {0};
    localtime_r(&currentTime, &currentTimeData);
    struct tm lastTimeData = {0};
    localtime_r(&lastWriteTime, &lastTimeData);
    if (difftime(currentTime, lastWriteTime) > ONE_DAY_TO_SECONDS ||
        (currentTimeData.tm_year == lastTimeData.tm_year && currentTimeData.tm_mon == lastTimeData.tm_mon &&
        currentTimeData.tm_mday - lastTimeData.tm_mday >= 1)) {
        DUMPER_HILOGI(MODULE_COMMON, "start clear data");
        std::ofstream ofs(HISYSEVENT_TMP_FILE, std::ios::out | std::ios::trunc);
        ofs.close();
    }
}

uint64_t DumpCommonUtils::GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

void DumpCommonUtils::GetDateAndTime(uint64_t timeStamp, std::string &dateTime)
{
    time_t time = static_cast<time_t>(timeStamp);
    struct tm timeData = {0};
    localtime_r(&time, &timeData);

    dateTime = "";
    dateTime.append(std::to_string(TM_START_YEAR + timeData.tm_year));
    dateTime.append("-");
    if (1 + timeData.tm_mon < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(1 + timeData.tm_mon));
    dateTime.append("-");
    if (timeData.tm_mday < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_mday));
    dateTime.append(" ");
    if (timeData.tm_hour < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_hour));
    dateTime.append(":");
    if (timeData.tm_min < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_min));
    dateTime.append(":");
    if (timeData.tm_sec < DEC_SYSTEM_VALUE) {
        dateTime.append(std::to_string(0));
    }
    dateTime.append(std::to_string(timeData.tm_sec));
}

} // namespace HiviewDFX
} // namespace OHOS
