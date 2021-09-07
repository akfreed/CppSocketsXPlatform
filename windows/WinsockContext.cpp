// ==================================================================
// Copyright 2018 Alexander K. Freed
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

#include "WinsockContext.h"

#include <string>
#include <stdexcept>
#include <mutex>

namespace {

std::mutex& GetLockInstance()
{
    static std::mutex s_instance;
    return s_instance;
}

}

WinsockContext::WinsockContext()
{
    std::lock_guard<std::mutex> lock(GetLockInstance());
    WSADATA wsaData{};
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
        throw std::runtime_error("Unable to initializer Winsock. WSA error: " + std::to_string(result));
}

WinsockContext::~WinsockContext()
{
    std::lock_guard<std::mutex> lock(GetLockInstance());
    WSACleanup();
}
