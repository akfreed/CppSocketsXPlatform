//============================================================================
// Copyright (c) 2018 Alexander Freed
//
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
//============================================================================
#pragma once

#include "SocketIncludes.h"

#include <mutex>
#include <condition_variable>

#include "WinsockContextManager.h"


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

        void move(UdpSocket&& other) noexcept(false);
        void close(std::unique_lock<std::mutex>& lock);
        bool checkSenderInfo(const sockaddr_in& info, int len, std::unique_lock<std::mutex>&) const;
        void setSenderInfo(sockaddr_in& info, int len, std::unique_lock<std::mutex>&);
        bool preReadSetup();
        bool postReadCheck(int amountRead, int maxlen, sockaddr_in& info, int infoLen, std::unique_lock<std::mutex>& lock);

        SOCKET      m_socketId;
        sockaddr_in m_myInfo;
        sockaddr_in m_theirInfo;
        int         m_theirInfoLen;
        bool        m_theirInfoIsValid;
        State       m_state;
        std::mutex  m_socketLock;
        std::condition_variable m_readCancel;
        int         m_dumpCount;
};

