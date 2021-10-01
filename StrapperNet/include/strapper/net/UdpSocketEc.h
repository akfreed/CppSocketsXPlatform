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

#include <strapper/net/UdpSocket.h>

namespace strapper { namespace net {

class ErrorCode;

class UdpSocketEc
{
public:
    UdpSocketEc() = default;
    explicit UdpSocketEc(uint16_t myport, ErrorCode* ec);

    bool IsOpen() const;
    void SetReadTimeout(unsigned milliseconds, ErrorCode* ec);

    void Close();

    void Write(void const* src, size_t len, IpAddressV4 const& ipAddress, uint16_t port, ErrorCode* ec);
    unsigned Read(void* dest, size_t maxlen, IpAddressV4* out_ipAddress, uint16_t* out_port, ErrorCode* ec);

    unsigned DataAvailable(ErrorCode* ec) const;

    explicit operator bool() const;

    UdpSocket&& Convert();

private:
    UdpSocket m_socket;
};

} }
