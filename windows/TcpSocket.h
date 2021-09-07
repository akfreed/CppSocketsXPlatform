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

// Contains the declaration TcpSocket
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

class TcpSocket
{
public:
    TcpSocket() = default;
    TcpSocket(std::string const& host, uint16_t port);
    TcpSocket(TcpSocket const&) = delete;
    TcpSocket(TcpSocket&& other) noexcept(false);
    TcpSocket& operator=(TcpSocket const&) = delete;
    TcpSocket& operator=(TcpSocket&& other) noexcept(false);
    ~TcpSocket();

    bool IsConnected() const;
    bool SetReadTimeout(unsigned milliseconds);

    bool Connect(std::string const& host, uint16_t port);
    void Close();

    bool Write(char const* buf, int len);
    bool Read(char* buf, int len);

    int DataAvailable() const;

    class Attorney
    {
        friend class TcpListener;
        static TcpSocket accept(SOCKET fd) { return TcpSocket(fd); }
    };

private:
    enum class State
    {
        CONNECTED,
        READING,
        SHUTTING_DOWN,
        CLOSED
    };

    explicit TcpSocket(SOCKET fd);

    void move(TcpSocket&& other) noexcept(false);
    void close(std::unique_lock<std::mutex>& lock);
    bool preReadSetup();
    bool postReadCheck(int amountRead, int len);

    WinsockContext m_winsockContext;
    SOCKET m_socketId = INVALID_SOCKET;
    addrinfo m_hostInfo{};
    addrinfo* m_hostInfoList = nullptr;
    State m_state = State::CLOSED;
    mutable std::mutex m_socketLock;
    std::condition_variable m_readCancel;
};
