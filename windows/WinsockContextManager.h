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

// Provides a very simple way to initialize winsock and ensure that cleanup
// happens. Just declare a WinsockContextManager object in the scope (or 
// higher) you want Winsock.
//
// This file should be included before windows.h

#pragma once

#include "SocketIncludes.h"

#include <assert.h>


class WinsockContextManager
{
public:
    WinsockContextManager()
        : m_wsaDataValid(false)
    {
        if (!isInstance())
        {
            // Initialize Winsock
            if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
                return;
            m_wsaDataValid = true;
        }
        addInstance();
    }

    ~WinsockContextManager()
    {
        subInstance();
        if (!isInstance())
            WSACleanup();
    }

    WinsockContextManager(const WinsockContextManager&) = delete;
    WinsockContextManager(WinsockContextManager&&) = delete;
    WinsockContextManager& operator=(const WinsockContextManager&) = delete;
    WinsockContextManager& operator=(WinsockContextManager&&) = delete;

    bool IsInitialized()
    {
        return isInstance();
    }

private:
    // d will add or subtract its amount to the instance count
    static int instanceCount(int d)
    {
        static unsigned instanceCount = 0;
        instanceCount += d;
        if (instanceCount < 0)
        {
            assert(false);
            instanceCount = 0;
        }
        return instanceCount;
    }

    void addInstance()
    {
        instanceCount(1);
    }
    void subInstance()
    {
        instanceCount(-1);
    }
    bool isInstance()
    {
        return (bool)instanceCount(0);
    }

    WSADATA m_wsaData;
    bool    m_wsaDataValid;
};
