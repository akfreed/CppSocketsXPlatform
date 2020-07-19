//============================================================================
// Copyright (c) 2018 Alexander Freed
//
// Provides a very simple way to initialize winsock and ensure that cleanup
// happens. Just declare a WinsockContextManager object in the scope (or 
// higher) you want Winsock.
//
// This file should be included before windows.h
//============================================================================
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
