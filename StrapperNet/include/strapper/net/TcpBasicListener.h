// ==================================================================
// Copyright 2018-2021 Alexander K. Freed
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

#include <strapper/net/SocketHandle.h>
#include <strapper/net/SystemContext.h>
#include <strapper/net/TcpBasicSocket.h>

#include <cstdint>

namespace strapper { namespace net {

class TcpBasicListener
{
    friend class TcpListener;  // todo: remove
public:
    TcpBasicListener() = default;
    explicit TcpBasicListener(uint16_t port);
    TcpBasicListener(TcpBasicListener const&) = delete;
    TcpBasicListener(TcpBasicListener&&) = default;
    TcpBasicListener& operator=(TcpBasicListener const&) = delete;
    TcpBasicListener& operator=(TcpBasicListener&&) = default;
    ~TcpBasicListener();

    bool IsListening() const;

    void Close() noexcept;
    TcpBasicSocket Accept();

    explicit operator bool() const;

private:
    void shutdown() noexcept;

    SystemContext m_context;
    SocketHandle m_socket;
};

}}  // namespace strapper::net
