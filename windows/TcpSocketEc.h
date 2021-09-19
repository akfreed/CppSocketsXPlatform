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

#include <TcpSocket.h>

namespace strapper { namespace net {

class ErrorCode;

class TcpSocketEc
{
public:
    TcpSocketEc() = default;
    TcpSocketEc(std::string const& host, uint16_t port, ErrorCode* ec);
    explicit TcpSocketEc(TcpSocket&& socket);

    bool IsConnected() const;
    void SetReadTimeout(unsigned milliseconds, ErrorCode* ec);

    void ShutdownSend(ErrorCode* ec);
    void ShutdownBoth();
    void Close();

    void Write(void const* src, size_t len, ErrorCode* ec);
    bool Read(void* dest, size_t len, ErrorCode* ec);

    unsigned DataAvailable(ErrorCode* ec);

    explicit operator bool() const;

    TcpSocket&& Convert();

private:
    TcpSocket m_socket;
};

} }
