// ==================================================================
// Copyright 2021 Alexander K. Freed
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

#include <TcpListener.h>
#include <TcpSocketEc.h>

namespace strapper { namespace net {

class ErrorCode;

class TcpListenerEc
{
public:
    TcpListenerEc() = default;
    explicit TcpListenerEc(uint16_t port, ErrorCode* ec);

    bool IsListening() const;

    void Close();
    TcpSocketEc Accept(ErrorCode* ec);

    explicit operator bool() const;

    TcpListener&& Convert();

private:
    TcpListener m_listener;
};

} }
