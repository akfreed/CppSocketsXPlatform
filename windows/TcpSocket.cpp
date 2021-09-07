// ==================================================================
// Copyright 2018-2021 Alexander K. Freed
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

// Contains the cpp code for TcpSocket

#include "TcpSocket.h"

#include <cassert>

// constructor connects to host:port
TcpSocket::TcpSocket(const char* host, const char* port)
{
    Connect(host, port);
}

// default constructor
// Socket is not connected
TcpSocket::TcpSocket()
{ 
    std::lock_guard<std::mutex> lock(m_socketLock);  // ensures proper memory fencing
}

// special private constructor used only by TcpListener.Accept(), which has a friend function
TcpSocket::TcpSocket(SOCKET fd)
    : m_socketId(fd)
    , m_state(State::CONNECTED)
{
    std::lock_guard<std::mutex> lock(m_socketLock);  // ensures proper memory fencing
}

// move constructor
// Cannot be noexcept because it a mutex can fail to lock.
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.
TcpSocket::TcpSocket(TcpSocket&& other) noexcept(false)
    : TcpSocket()
{
    this->move(std::move(other));
}

// move assignment
// Cannot be noexcept because it a mutex can fail to lock.
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.
TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept(false)
{
    // move function handles cleaning up
    this->move(std::move(other));
    return *this;
}

// destructor
// Close() can throw, but in that case we pretty much have to abort anyways.
TcpSocket::~TcpSocket()
{
    Close();
}

//----------------------------------------------------------------------------

void TcpSocket::move(TcpSocket&& other) noexcept(false)
{
    std::lock(m_socketLock, other.m_socketLock);  // deadlock-proof
    std::unique_lock<std::mutex> lock(m_socketLock, std::adopt_lock);
    std::unique_lock<std::mutex> other_lock(other.m_socketLock, std::adopt_lock);

    // clean up
    close(lock);

    // If the other socket is reading, moving it probably create a logic bug.
    // If the other socket is shutting down, wait for it to finish.
    if (other.m_state == State::READING || other.m_state == State::SHUTTING_DOWN)
    {
        assert(other.m_state != State::READING);
        other.close(other_lock);
    }

    m_socketId = std::move(other.m_socketId);
    other.m_socketId = INVALID_SOCKET;

    m_hostInfo = std::move(other.m_hostInfo);
    ZeroMemory(&other.m_hostInfo, sizeof(other.m_hostInfo));

    m_hostInfoList = std::move(other.m_hostInfoList);
    other.m_hostInfoList = nullptr;

    m_state = std::move(other.m_state);
    other.m_state = State::CLOSED;
}

// shutdown and close the socket
void TcpSocket::Close()
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    close(lock);
}

// be sure to lock before calling!
void TcpSocket::close(std::unique_lock<std::mutex>& lock)
{
    switch (m_state)
    {
    case State::CLOSED:
        return;  // return!
        break;

    case State::SHUTTING_DOWN:
        while (m_state != State::CLOSED)
            m_readCancel.wait(lock);
        return;  // return!
        break;

    case State::CONNECTED:
        m_state = State::CLOSED;
        shutdown(m_socketId, SD_BOTH);
        break;  // fall out

    case State::READING:
        m_state = State::SHUTTING_DOWN;
        shutdown(m_socketId, SD_BOTH);
        // If there is a blocked read on a separate thread, the socket shutdown will unblock it. Wait for it to finish.
        while (m_state != State::CLOSED)
            m_readCancel.wait(lock);
        break;  // fall out
    }

    closesocket(m_socketId);
    m_socketId = INVALID_SOCKET;
    freeaddrinfo(m_hostInfoList);
}

// connects to host:port
bool TcpSocket::Connect(const char* host, const char* port)
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state != State::CLOSED)
        return false;

    ZeroMemory(&m_hostInfo, sizeof(m_hostInfo));
    m_hostInfo.ai_family = AF_UNSPEC;  // can be IPv4 or IPv6
    m_hostInfo.ai_socktype = SOCK_STREAM;  // for TCP

                                           // get some info. Fill the struct linked list
    int error = getaddrinfo(host, port, &m_hostInfo, &m_hostInfoList);
    if (error != 0)
    {
        return false;
    }

    // get us a socket
    m_socketId = socket(m_hostInfoList->ai_family, m_hostInfoList->ai_socktype, m_hostInfoList->ai_protocol);
    if (m_socketId == INVALID_SOCKET)
    {
        freeaddrinfo(m_hostInfoList);
        m_hostInfoList = nullptr;
        return false;
    }

    // connect
    error = connect(m_socketId, m_hostInfoList->ai_addr, (int)m_hostInfoList->ai_addrlen);
    if (error == SOCKET_ERROR)
    {
        freeaddrinfo(m_hostInfoList);
        m_hostInfoList = nullptr;
        closesocket(m_socketId);
        return false;
    }

    m_state = State::CONNECTED;
    return true;
}

//----------------------------------------------------------------------------

bool TcpSocket::IsConnected()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    return m_state != State::CLOSED;
}

//----------------------------------------------------------------------------

// If this timeout is reached, the socket will be closed. Thank you, Windows.
// Only use this feature as a robustness mechanism.
// (e.g. so you don't block forever if the connection is somehow silently lost.)
// Don't use this as a form of non-blocking read.
bool TcpSocket::SetReadTimeout(unsigned milliseconds)
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state != State::CONNECTED)
        return false;
    return setsockopt(m_socketId, SOL_SOCKET, SO_RCVTIMEO, (const char*)&milliseconds, sizeof(milliseconds)) == 0;
}

//----------------------------------------------------------------------------

bool TcpSocket::Write(const char* buf, int len)
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    if (m_state == State::CLOSED || m_state == State::SHUTTING_DOWN)
        return false;

    if (len <= 0)
    {
        assert(false);
        return true;
    }

    int amountWritten = send(m_socketId, buf, len, 0);

    if (amountWritten == SOCKET_ERROR)
    {
        close(lock);
        return false;
    }
    return true;
}

bool TcpSocket::preReadSetup()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    // Sanity check.
    if (m_state == State::READING)
    {
        assert(false);
        return false;
    }
    if (m_state != State::CONNECTED)
        return false;
    m_state = State::READING;
    return true;
}

bool TcpSocket::postReadCheck(int amountRead, int len)
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    if (m_state == State::SHUTTING_DOWN)
    {
        m_state = State::CLOSED;
        m_readCancel.notify_all();
        return false;
    }
    else if (amountRead == SOCKET_ERROR || amountRead < len)  // amount read can be less than len if a read timeout occurs, in which case we need to close the socket.
    {
        m_state = State::CONNECTED;
        close(lock);
        return false;
    }
    else
    {
        m_state = State::CONNECTED;
        return true;
    }
}

// reads len bytes into given char* buffer
bool TcpSocket::Read(char* buf, int len)
{
    // sanity check
    if (len < 1)
    {
        assert(false);
        return false;
    }

    if (!preReadSetup())
        return false;

    int amountRead = recv(m_socketId, buf, len, MSG_WAITALL);  // MSG_WAITALL blocks until all bytes are there
    
    return postReadCheck(amountRead, len);
}

// returns the amount of bytes available in the stream
// guaranteed not to be bigger than the actual number
// you can read this many bytes without blocking
// may be smaller than the actual number of bytes available
// returns -1 on error
int TcpSocket::DataAvailable()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state != State::CONNECTED)
    {
        assert(false);
        return -1;
    }

    unsigned long bytesAvailable;
    if (ioctlsocket(m_socketId, FIONREAD, &bytesAvailable) != 0)
        return -1;

    if (bytesAvailable > INT_MAX)
        bytesAvailable = INT_MAX;

    return static_cast<int>(bytesAvailable);
}
