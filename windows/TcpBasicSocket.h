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

#include <WinsockContext.h>
#include <SocketHandle.h>

#include <cstdint>
#include <string>

namespace strapper { namespace net {

class TcpBasicSocket
{
public:
    TcpBasicSocket() = default;
    TcpBasicSocket(std::string const& host, uint16_t port);
    TcpBasicSocket(TcpBasicSocket const&) = delete;
    TcpBasicSocket(TcpBasicSocket&&) = default;
    TcpBasicSocket& operator=(TcpBasicSocket const&) = delete;
    TcpBasicSocket& operator=(TcpBasicSocket&&) = default;
    ~TcpBasicSocket();

    bool IsConnected() const;
    void SetReadTimeout(unsigned milliseconds);

    void ShutdownSend();
    void ShutdownReceive();
    void ShutdownBoth() noexcept;
    void Close() noexcept;

    void Write(void const* src, size_t len);
    bool Read(void* dest, size_t len);

    unsigned DataAvailable();

    explicit operator bool() const;

    class Attorney
    {
        friend class TcpBasicListener;
        static TcpBasicSocket accept(SocketHandle&& socket) { return TcpBasicSocket(std::move(socket)); }
    };

private:
    explicit TcpBasicSocket(SocketHandle&& socket);

    WinsockContext m_winsockContext;
    SocketHandle m_socket;
};

} }
