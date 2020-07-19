//============================================================================
// Copyright (c) 2018 Alexander Freed
//
// Contains the cpp code for TcpListener
//============================================================================

#include "TcpListener.h"
#include "TcpSocket.h"

#include <cstring>   // memset
#include <netdb.h>
#include <unistd.h>  // close
#include <assert.h>


TcpListener::TcpListener(const char* port)
    : m_socketId(INVALID_SOCKET)
    , m_state(State::CLOSED)
{
    start(port);
}


// Cannot be noexcept because it a mutex can fail to lock.
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.
TcpListener::TcpListener(TcpListener&& other) noexcept(false)
    : m_socketId(INVALID_SOCKET)
    , m_state(State::CLOSED)
{
    this->move(std::move(other));
}


// Cannot be noexcept because it a mutex can fail to lock.
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.
TcpListener& TcpListener::operator=(TcpListener&& other) noexcept(false)
{
    // move function handles cleaning up
    this->move(std::move(other));
    return *this;
}


// Close() can throw, but in that case we pretty much have to abort anyways.
TcpListener::~TcpListener() noexcept
{
    Close();
}


//----------------------------------------------------------------------------

void TcpListener::move(TcpListener&& other) noexcept(false)
{    
    std::lock(m_lock, other.m_lock);  // deadlock-proof
    std::unique_lock<std::mutex> lock(m_lock, std::adopt_lock);
    std::unique_lock<std::mutex> other_lock(other.m_lock, std::adopt_lock);

    // clean up
    close(lock);

    // If the other listener is accpting, moving it probably create a logic bug.
    // If the other listener is shutting down, wait for it to finish.
    if (other.m_state == State::ACCEPTING || other.m_state == State::SHUTTING_DOWN)
    {
        assert(other.m_state != State::ACCEPTING);
        other.close(other_lock);
    }

    m_socketId = std::move(other.m_socketId);
    other.m_socketId = INVALID_SOCKET;

    m_state = std::move(other.m_state);
    other.m_state = State::CLOSED;
}


void TcpListener::Close()
{
    std::unique_lock<std::mutex> lock(m_lock);
    close(lock);
}


// Be sure to lock before calling!
void TcpListener::close(std::unique_lock<std::mutex>& lock)
{
    switch (m_state)
    {
    case State::CLOSED:
        return;  // return!
        break;

    case State::SHUTTING_DOWN:
        while (m_state != State::CLOSED)
            m_acceptCancel.wait(lock);
        return;  // return!
        break;

    case State::OPEN:
        m_state = State::CLOSED;
        shutdown(m_socketId, SHUT_RDWR);
        break;  // fall out

    case State::ACCEPTING:
        m_state = State::SHUTTING_DOWN;
        shutdown(m_socketId, SHUT_RDWR);
        while (m_state != State::CLOSED)
            m_acceptCancel.wait(lock);
        break;  // fall out
    }
    
    ::close(m_socketId);
    m_socketId = INVALID_SOCKET;
}


bool TcpListener::start(const char* port)
{
    std::lock_guard<std::mutex> lock(m_lock);
    if (m_state != State::CLOSED)
        return false;
    
    addrinfo hostInfo;
    addrinfo* hostInfoList = nullptr;

    memset(&hostInfo, 0, sizeof(hostInfo));
    hostInfo.ai_family = AF_INET;
    hostInfo.ai_socktype = SOCK_STREAM;
    hostInfo.ai_flags = AI_PASSIVE;
    int error = getaddrinfo(NULL, port, &hostInfo, &hostInfoList);
    if (error != 0)
    {
        return false;
    }

    m_socketId = socket(hostInfoList->ai_family, hostInfoList->ai_socktype, hostInfoList->ai_protocol);
    if (m_socketId == INVALID_SOCKET)
    {
        freeaddrinfo(hostInfoList);
        return false;
    }

    int yes = 1;
    error = setsockopt(m_socketId, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (error == -1)
    {
        freeaddrinfo(hostInfoList);
        ::close(m_socketId);
        m_socketId = -1;
        return false;
    }

    error = bind(m_socketId, hostInfoList->ai_addr, hostInfoList->ai_addrlen);
    if (error == -1)
    {
        freeaddrinfo(hostInfoList);
        ::close(m_socketId);
        m_socketId = INVALID_SOCKET;
        return false;
    }

    freeaddrinfo(hostInfoList);

    error = listen(m_socketId, 128);
    if (error == SOCKET_ERROR)
    {
        ::close(m_socketId);
        return false;
    }

    m_state = State::OPEN;
    return true;
}


//----------------------------------------------------------------------------

bool TcpListener::IsValid()
{
    std::lock_guard<std::mutex> lock(m_lock);  // lock used here mostly for the memory fence...
    return m_state != State::CLOSED;
}


// friend of class TcpSocket, allowing us to use a special constructor
TcpSocket TcpListener::Accept()
{
    {
        std::lock_guard<std::mutex> lock(m_lock);
        assert(m_state != State::ACCEPTING);  // sanity check
        if (m_state != State::OPEN)
            return TcpSocket();
        m_state = State::ACCEPTING;
    }

    int clientId = accept(m_socketId, NULL, NULL);  // todo: implement args 2 and 3
    
    {
        std::unique_lock<std::mutex> lock(m_lock);
        if (m_state == State::SHUTTING_DOWN)
        {
            if (clientId != INVALID_SOCKET)
                TcpSocket::Attorney::accept(clientId).Close();
            m_state = State::CLOSED;
            m_acceptCancel.notify_all();
            return TcpSocket();
        }
        else if (clientId == INVALID_SOCKET)
        {
            m_state = State::OPEN;
            return TcpSocket();
        }
        else
        {
            m_state = State::OPEN;
            return TcpSocket::Attorney::accept(clientId);
        }
    }
}
