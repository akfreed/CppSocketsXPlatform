// ==================================================================
// Copyright 2018-2022 Alexander K. Freed
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

#include "SocketIncludes.h"
#include "NativeContext.h"

#include <strapper/net/SocketError.h>

namespace strapper { namespace net {

std::mutex NativeContext::s_mutex;

std::shared_ptr<NativeContext> NativeContext::Get()
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

// Lock before calling.
NativeContext::NativeContext()
{
    WSADATA wsaData{};
    int const result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
        throw SocketError(result);
}

NativeContext::~NativeContext()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    WSACleanup();
}

}}  // namespace strapper::net
