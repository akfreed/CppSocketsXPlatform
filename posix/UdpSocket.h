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
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.

#pragma once

#include <netinet/in.h>
#include <mutex>
#include <condition_variable>

#include "WinsockContextManager.h"  // for compatibility with the Windows version


class UdpSocket
{
    public:
        UdpSocket();
        explicit UdpSocket(unsigned short myport);
        UdpSocket(const char* host, unsigned short hostport);
        UdpSocket(const char* host, unsigned short hostport, unsigned short myport);
        UdpSocket(const UdpSocket &) = delete;
        UdpSocket(UdpSocket &&) noexcept(false);
        UdpSocket& operator=(const UdpSocket&) = delete;
        UdpSocket& operator=(UdpSocket&&) noexcept(false);
        ~UdpSocket() noexcept;

        int GetDumpCount() const;
        bool IsValid();
        bool SetReadTimeout(unsigned milliseconds);
        int DataAvailable();

        void Close();
        bool DumpReadBuffer();

        bool Write(const char* source, int len);
        int Read(char* dest, int maxlen);

    private:
        enum class State
        {
            OPEN,
            READING,
            SHUTTING_DOWN,
            CLOSED
        };
        
        static const int INVALID_SOCKET = -1;
        static const int SOCKET_ERROR = -1;
        
        void move(UdpSocket&& other) noexcept(false);
        void close(std::unique_lock<std::mutex>& lock);

        bool checkSenderInfo(const sockaddr_in& info, int len, std::unique_lock<std::mutex>&) const;
        void setSenderInfo(sockaddr_in& info, int len, std::unique_lock<std::mutex>&);
        bool preReadSetup();
        bool postReadCheck(int amountRead, int maxlen, sockaddr_in&, int infoLen, std::unique_lock<std::mutex>& lock);

        int         m_socketId;
        sockaddr_in m_myInfo;
        sockaddr_in m_theirInfo;
        int         m_theirInfoLen;
        bool        m_theirInfoIsValid;
        State       m_state;
        std::mutex  m_socketLock;
        std::condition_variable m_readCancel;
        int         m_dumpCount;
};

