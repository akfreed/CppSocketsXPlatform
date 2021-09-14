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
#include <IpAddress.h>

#include <cstdint>

namespace strapper { namespace net {

class UdpSocketBase
{
public:
    UdpSocketBase();
    explicit UdpSocketBase(uint16_t myport);
    UdpSocketBase(UdpSocketBase const&) = delete;
    UdpSocketBase(UdpSocketBase&& other) = default;
    UdpSocketBase& operator=(UdpSocketBase const&) = delete;
    UdpSocketBase& operator=(UdpSocketBase&& other) = default;
    ~UdpSocketBase();

    bool IsOpen() const;
    void SetReadTimeout(unsigned milliseconds);

    void Close();

    void Write(void const* src, size_t len, IpAddressV4 const& ipAddress, uint16_t port);
    void Read(void* dest, size_t maxlen, IpAddressV4* out_ipAddress, uint16_t* out_port);

    unsigned DataAvailable() const;

    explicit operator bool() const;

private:    
    WinsockContext m_winsockContext;
    SocketHandle m_socket;
};

} }
