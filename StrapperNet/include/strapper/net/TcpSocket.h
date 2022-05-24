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

#pragma once

#include <strapper/net/TcpBasicSocket.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <utility>

namespace strapper { namespace net {

class ErrorCode;

class TcpSocket
{
public:
    TcpSocket() = default;
    TcpSocket(std::string const& host, uint16_t port, ErrorCode* ec = nullptr);
    TcpSocket(TcpSocket const&) = delete;
    TcpSocket(TcpSocket&& other) noexcept;
    TcpSocket& operator=(TcpSocket const&) = delete;
    TcpSocket& operator=(TcpSocket&& other) noexcept;
    ~TcpSocket();

    friend void swap(TcpSocket& left, TcpSocket& right);

    bool IsOpen() const;
    void SetReadTimeout(unsigned milliseconds, ErrorCode* ec = nullptr);

    void ShutdownSend(ErrorCode* ec = nullptr);
    void ShutdownBoth();
    void Close() noexcept;

    void Write(void const* src, size_t len, ErrorCode* ec = nullptr);
    bool Read(void* dest, size_t len, ErrorCode* ec = nullptr);

    unsigned DataAvailable(ErrorCode* ec = nullptr);

    explicit operator bool() const;

    class Attorney
    {
        friend class TcpListener;
        static TcpSocket accept(TcpBasicSocket&& socket) { return TcpSocket(std::move(socket)); }
    };

private:
    enum class State
    {
        CONNECTED,
        READING,
        SHUTTING_DOWN,
        CLOSED
    };

    explicit TcpSocket(TcpBasicSocket&& socket);

    bool read(void* dest, size_t len);

    mutable std::mutex m_socketLock;
    std::condition_variable m_readCancel;
    TcpBasicSocket m_socket;
    State m_state = State::CLOSED;
};

}}  // namespace strapper::net
