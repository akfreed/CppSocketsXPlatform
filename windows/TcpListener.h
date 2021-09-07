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

// Contains the declaration for TcpListener.
//
// This file should be included before windows.h
//
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.

#pragma once

#include <WinsockContext.h>

#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <string>

class TcpSocket;

class TcpListener
{
public:
    explicit TcpListener(uint16_t port);
    TcpListener(TcpListener const&) = delete;
    TcpListener(TcpListener&&) noexcept(false);
    TcpListener& operator=(TcpListener const&) = delete;
    TcpListener& operator=(TcpListener&&) noexcept(false);
    ~TcpListener();

    bool IsValid() const;

    void Close();
    TcpSocket Accept();

private:
    enum class State
    {
        OPEN,
        ACCEPTING,
        SHUTTING_DOWN,
        CLOSED
    };

    bool start(uint16_t port);
    void move(TcpListener&& other) noexcept(false);
    void close(std::unique_lock<std::mutex>& lock);

    WinsockContext m_winsockContext;
    SOCKET m_socketId = INVALID_SOCKET;
    State m_state = State::CLOSED;
    mutable std::mutex m_lock;
    std::condition_variable m_acceptCancel;
};
