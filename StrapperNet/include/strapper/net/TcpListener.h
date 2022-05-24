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

#include <strapper/net/TcpBasicListener.h>
#include <strapper/net/TcpSocket.h>

#include <condition_variable>
#include <mutex>
#include <utility>

namespace strapper { namespace net {

class ErrorCode;

class TcpListener
{
public:
    TcpListener() = default;
    explicit TcpListener(uint16_t port, ErrorCode* ec = nullptr);
    TcpListener(TcpListener const&) = delete;
    TcpListener(TcpListener&& other) noexcept;
    TcpListener& operator=(TcpListener const&) = delete;
    TcpListener& operator=(TcpListener&& other) noexcept;
    ~TcpListener();

    friend void swap(TcpListener& left, TcpListener& right);

    bool IsListening() const;

    void Close() noexcept;
    TcpSocket Accept(ErrorCode* ec = nullptr);

    explicit operator bool() const;

private:
    enum class State
    {
        OPEN,
        ACCEPTING,
        SHUTTING_DOWN,
        CLOSED
    };

    TcpSocket accept();

    mutable std::mutex m_lock;
    std::condition_variable m_acceptCancel;
    TcpBasicListener m_listener;
    State m_state = State::CLOSED;
};

}}  // namespace strapper::net
