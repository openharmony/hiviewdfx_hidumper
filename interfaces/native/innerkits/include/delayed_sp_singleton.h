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
#ifndef HIDUMPER_UTILS_SP_SINGLETON_H
#define HIDUMPER_UTILS_SP_SINGLETON_H
#include <mutex>
#include <refbase.h>
#include "nocopyable.h"
namespace OHOS {
namespace HiviewDFX {
template<typename T>
class DumpDelayedSpSingleton : public NoCopyable {
public:
    static sptr<T> GetInstance();
    static void DestroyInstance();
private:
    static sptr<T> instance_;
    static std::mutex mutex_;
};

template<typename T>
sptr<T> DumpDelayedSpSingleton<T>::instance_ = nullptr;

template<typename T>
std::mutex DumpDelayedSpSingleton<T>::mutex_;

template<typename T>
void DumpDelayedSpSingleton<T>::DestroyInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance_) {
        instance_.clear();
        instance_ = nullptr;
    }
}

template<typename T>
sptr<T> DumpDelayedSpSingleton<T>::GetInstance()
{
    if (!instance_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = new T();
        }
    }
    return instance_;
}
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIDUMPER_UTILS_SP_SINGLETON_H
