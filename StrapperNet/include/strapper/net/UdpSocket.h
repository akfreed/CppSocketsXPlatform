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

#include <strapper/net/UdpBasicSocket.h>

#include <mutex>
#include <condition_variable>

namespace strapper { namespace net {

class UdpSocket
{
public:
    UdpSocket() = default;
    explicit UdpSocket(uint16_t myport);
    UdpSocket(UdpSocket const&) = delete;
    UdpSocket(UdpSocket&& other) noexcept;
    UdpSocket& operator=(UdpSocket const&) = delete;
    UdpSocket& operator=(UdpSocket&& other) noexcept;
    ~UdpSocket();

    friend void swap(UdpSocket& left, UdpSocket& right);

    bool IsOpen() const;
    void SetReadTimeout(unsigned milliseconds);

    void Close() noexcept;

    void Write(void const* src, size_t len, IpAddressV4 const& ipAddress, uint16_t port);
    unsigned Read(void* dest, size_t maxlen, IpAddressV4* out_ipAddress, uint16_t* out_port);

    unsigned DataAvailable() const;

    explicit operator bool() const;

private:
    enum class State
    {
        OPEN,
        READING,
        SHUTTING_DOWN,
        CLOSED
    };

    mutable std::mutex m_socketLock;
    std::condition_variable m_readCancel;
    UdpBasicSocket m_socket;
    State m_state = State::CLOSED;
};

} }