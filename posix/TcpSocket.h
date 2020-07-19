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

// Contains the declaration TcpSocket
//
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.

#pragma once

#include <netdb.h>
#include <mutex>
#include <condition_variable>

#include "TcpListener.h"
#include "WinsockContextManager.h"  // for compatibility with the Windows version

/*
Provides a cross platform wrapper functions for sockets
This is the wrapper for Linux
*/

class TcpSocket
{
    public:
        static const int MAX_STRING_LEN = 4096;

        TcpSocket();
        TcpSocket(const char* host, const char* port);
        TcpSocket(const TcpSocket&) = delete;
        TcpSocket(TcpSocket&&) noexcept(false);
        TcpSocket& operator=(const TcpSocket&) = delete;
        TcpSocket& operator=(TcpSocket&&) noexcept(false);
        ~TcpSocket() noexcept;
        
        int GetDumpCount() const;
        bool IsConnected();
        bool SetReadTimeout(unsigned milliseconds);
        
        bool Connect(const char* host, const char* port);
        void Close();

        bool Write(char c);
        bool Write(bool b);
        bool Write(int32_t int32);
        bool Write(double d);
        bool Write(const char*, int len);
        bool WriteString(const char* str);
        bool Read(char &dest);
        bool Read(bool &dest);
        bool Read(int32_t &dest);
        bool Read(double &dest);
        bool Read(char* dest, int len);
        int ReadString(char* c, int maxlen);

        int DataAvailable();
        bool DumpReadBuffer();
        int DumpReadBuffer(int amountToDump);
        
        // Attorney class allows TcpListener to do essential operations without giving full access to private members of this class
        friend class Attorney;
        class Attorney
        {
        private:
            friend class TcpListener;
            static TcpSocket accept(int fd)
            {
                return TcpSocket(fd);
            }
        };

    private:
        enum class State
        {
            CONNECTED,
            READING,
            SHUTTING_DOWN,
            CLOSED
        };
        
        static const int INVALID_SOCKET = -1;
        static const int SOCKET_ERROR = -1;
        
        explicit TcpSocket(int fd);  // special constructor used by TcpListener::Accept()
        
        void move(TcpSocket&&) noexcept(false);
        void close(std::unique_lock<std::mutex>& lock);
        bool preReadSetup();
        bool postReadCheck(int amountRead, int len);
        
        int        m_socketId;
        addrinfo   m_hostInfo;
        addrinfo*  m_hostInfoList;
        State      m_state;
        std::mutex m_socketLock;
        std::condition_variable m_readCancel;
        int        m_dumpCount;
};

