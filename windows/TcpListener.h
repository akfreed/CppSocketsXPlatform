//============================================================================
// Copyright (c) 2018 Alexander Freed
//
// Contains the declaration for TcpListener.
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

        bool start(const char* port);
        void move(TcpListener&&) noexcept(false);
        void close(std::unique_lock<std::mutex>& lock);

        SOCKET     m_socketId;
        State      m_state;
        std::mutex m_lock;
        std::condition_variable m_acceptCancel;
};
