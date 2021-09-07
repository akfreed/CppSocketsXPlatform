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

// Contains the declaration for UdpSocket
// In the current implementation, a UdpSocket either sets its recipient at
// instantiation, or the first sender it reads from will be locked in as the
// recipient. If data is read from anyone who is not the expected sender, the
// read ignore the datagram and try again.
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

class UdpSocket
{
public:
    UdpSocket() = default;
    explicit UdpSocket(uint16_t myport);
    UdpSocket(std::string const& host, uint16_t hostport);
    UdpSocket(std::string const& host, uint16_t hostport, uint16_t myport);
    UdpSocket(UdpSocket const&) = delete;
    UdpSocket(UdpSocket&& other) noexcept(false);
    UdpSocket& operator=(UdpSocket const&) = delete;
    UdpSocket& operator=(UdpSocket&& other) noexcept(false);
    ~UdpSocket();

    bool IsValid() const;
    bool SetReadTimeout(unsigned milliseconds);
    int DataAvailable() const;

    void Close();

    bool Write(char const* source, int len);
    int Read(char* dest, int maxlen);

private:
    enum class State
    {
        OPEN,
        READING,
        SHUTTING_DOWN,
        CLOSED
    };

    void move(UdpSocket&& other) noexcept(false);
    void close(std::unique_lock<std::mutex>& lock);
    bool checkSenderInfo(sockaddr_in const& info, int len, std::unique_lock<std::mutex>&) const;
    void setSenderInfo(sockaddr_in& info, int len, std::unique_lock<std::mutex>&);
    bool preReadSetup();
    bool postReadCheck(int amountRead, int maxlen, sockaddr_in& info, int infoLen, std::unique_lock<std::mutex>& lock);

    WinsockContext m_winsockContext;
    SOCKET m_socketId = INVALID_SOCKET;
    sockaddr_in m_myInfo{};
    sockaddr_in m_theirInfo{};
    int m_theirInfoLen = sizeof(m_theirInfo);
    bool m_theirInfoIsValid = false;
    State m_state = State::CLOSED;
    mutable std::mutex m_socketLock;
    std::condition_variable m_readCancel;
};
