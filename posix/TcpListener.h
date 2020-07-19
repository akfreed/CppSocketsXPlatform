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
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.

#pragma once

#include <mutex>
#include <condition_variable>

#include "WinsockContextManager.h"  // for compatibility with the Windows version


class TcpSocket;

class TcpListener
{
    public:        
        explicit TcpListener(const char* port);
        TcpListener() = delete;
        TcpListener(const TcpListener&) = delete;
        TcpListener(TcpListener&&) noexcept(false);
        TcpListener& operator=(const TcpListener&) = delete;
        TcpListener& operator=(TcpListener&&) noexcept(false);
        ~TcpListener() noexcept;
        
        bool IsValid();
        
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
        
        static const int INVALID_SOCKET = -1;
        static const int SOCKET_ERROR = -1;
        
        bool start(const char* port);
        void move(TcpListener&&) noexcept(false);
        void close(std::unique_lock<std::mutex>& lock);
        
        int        m_socketId;
        State      m_state;
        std::mutex m_lock;
        std::condition_variable m_acceptCancel;
};
