//============================================================================
// Copyright (c) 2018 Alexander Freed
//
// Contains the declaration TcpSocket
// 
// This file should be included before windows.h
//
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.
//============================================================================
#pragma once

#include "SocketIncludes.h"

#include <stdint.h>
#include <mutex>
#include <condition_variable>

#include "TcpListener.h"
#include "WinsockContextManager.h"


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
        bool Write(const char* buf, int len);
        bool WriteString(const char* str);
        bool Read(char &dest);
        bool Read(bool &dest);
        bool Read(int32_t &dest);
        bool Read(double &dest);
        bool Read(char* buf, int len);
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
            static TcpSocket accept(SOCKET fd)
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

        explicit TcpSocket(SOCKET fd);  // special constructor used by TcpListener::Accept()

        void move(TcpSocket&&) noexcept(false);
        void close(std::unique_lock<std::mutex>& lock);
        bool preReadSetup();
        bool postReadCheck(int amountRead, int len);

        SOCKET     m_socketId;
        addrinfo   m_hostInfo;
        addrinfo*  m_hostInfoList;
        State      m_state;
        std::mutex m_socketLock;
        std::condition_variable m_readCancel;
        int        m_dumpCount;
};


