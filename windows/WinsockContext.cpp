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

#include <WinsockContext.h>

#include <string>
#include <stdexcept>
#include <mutex>

namespace strapper { namespace net {

class Winsock
{
public:
    static std::shared_ptr<Winsock> Get()
    {
        static std::weak_ptr<Winsock> s_instance;

        std::lock_guard<std::mutex> lock(s_mutex);

        std::shared_ptr<Winsock> winsock = s_instance.lock();
        if (!winsock)
        {
            winsock.reset(new Winsock());
            s_instance = winsock;
        }
        return winsock;
    }

    Winsock(Winsock const&) = delete;
    Winsock(Winsock&&) = delete;
    Winsock& operator=(Winsock const&) = delete;
    Winsock& operator=(Winsock&&) = delete;

    ~Winsock()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        WSACleanup();
    }

private:
    // Lock before calling.
    Winsock()
    {
        WSADATA wsaData{};
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0)
            throw std::runtime_error("Unable to initializer Winsock. WSA error: " + std::to_string(result));
    }

    static std::mutex s_mutex;
};

std::mutex Winsock::s_mutex;

WinsockContext::WinsockContext()
    : m_handle(Winsock::Get())
{ }

} }
