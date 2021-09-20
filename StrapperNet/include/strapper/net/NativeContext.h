// ==================================================================
// Copyright 2018, 2021 Alexander K. Freed
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ==================================================================

#pragma once

#include <memory>
#include <mutex>

namespace strapper { namespace net {

class NativeContext
{
public:
    static std::shared_ptr<NativeContext> Get()
    {
        static std::weak_ptr<NativeContext> s_instance;

        std::lock_guard<std::mutex> lock(s_mutex);

        std::shared_ptr<NativeContext> context = s_instance.lock();
        if (!context)
        {
            context.reset(new NativeContext());
            s_instance = context;
        }
        return context;
    }

    NativeContext(NativeContext const&) = delete;
    NativeContext(NativeContext&&) = delete;
    NativeContext& operator=(NativeContext const&) = delete;
    NativeContext& operator=(NativeContext&&) = delete;

    ~NativeContext();

private:
    NativeContext();

    static std::mutex s_mutex;
};

class SystemContext
{
private:
    std::shared_ptr<NativeContext> m_context = NativeContext::Get();
};

} }
