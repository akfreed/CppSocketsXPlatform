// ==================================================================
// Copyright 2018, 2021 Alexander K. Freed
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

#include <strapper/net/TcpSocket.h>

#include <strapper/net/SocketError.h>

#include <cassert>
#include <cstring>

namespace strapper { namespace net {

// constructor connects to host:port
TcpSocket::TcpSocket(std::string const& host, uint16_t port)
    : m_socket(host, port)
    , m_state(State::CONNECTED)
{
    assert(m_socket);
}

// special private constructor used only by TcpListener.Accept(), which has a friend function
TcpSocket::TcpSocket(TcpBasicSocket&& socket)
    : m_socket(std::move(socket))
    , m_state(m_socket ? State::CONNECTED : State::CLOSED)
{ }

TcpSocket::TcpSocket(TcpSocket&& other) noexcept
    : TcpSocket()
{
    swap(*this, other);
}

TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept
{
    TcpSocket temp(std::move(other));
    swap(*this, temp);
    return *this;
}

TcpSocket::~TcpSocket()
{
    Close();
}

void swap(TcpSocket& left, TcpSocket& right)
{
    using State = TcpSocket::State;
    std::lock(left.m_socketLock, right.m_socketLock); // Avoids deadlock.
    std::unique_lock<std::mutex> leftLock(left.m_socketLock, std::adopt_lock);
    std::unique_lock<std::mutex> rightLock(right.m_socketLock, std::adopt_lock);

    // If the socket is reading, moving it probably create a logic bug.
    assert(left.m_state != State::READING);
    assert(right.m_state != State::READING);
    // If the socket is shutting down, wait for it to finish.
    if (left.m_state == State::SHUTTING_DOWN)
        left.m_readCancel.wait(leftLock, [&left]() { return left.m_state == State::CLOSED; });
    if (right.m_state == State::SHUTTING_DOWN)
        right.m_readCancel.wait(rightLock, [&right]() { return right.m_state == State::CLOSED; });

    using std::swap;
    swap(left.m_socket, right.m_socket);
    swap(left.m_state, right.m_state);
}

bool TcpSocket::IsConnected() const
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    return m_state != State::CLOSED;
}

//! If this timeout is reached, the socket will be closed. Thank you, Windows.
//! Only use this feature as a robustness mechanism.
//! (e.g. so you don't block forever if the connection is somehow silently lost.)
//! Don't use this as a form of non-blocking read.
void TcpSocket::SetReadTimeout(unsigned milliseconds)
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state == State::CLOSED)
        throw ProgramError("Socket is not connected.");
    if (m_state == State::READING)
        throw ProgramError("Socket is already reading.");
    if (m_state == State::SHUTTING_DOWN)
        throw ProgramError("Socket was closed from another thread.");

    m_socket.SetReadTimeout(milliseconds);
}

void TcpSocket::ShutdownSend()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state == State::CLOSED)
        throw ProgramError("Socket is not connected.");
    if (m_state == State::SHUTTING_DOWN)
        throw ProgramError("Socket was closed from another thread.");

    m_socket.ShutdownSend();
}

void TcpSocket::ShutdownBoth()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state == State::CLOSED)
        throw ProgramError("Socket is not connected.");
    if (m_state == State::SHUTTING_DOWN)
        throw ProgramError("Socket was closed from another thread.");

    m_socket.ShutdownBoth();
}

// Shutdown and close the socket.
void TcpSocket::Close() noexcept
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    switch (m_state)
    {
    case State::CLOSED:
        break;

    case State::SHUTTING_DOWN:
        m_readCancel.wait(lock, [this]() { return m_state == State::CLOSED; });
        break;

    case State::CONNECTED:
        m_state = State::CLOSED;
        m_socket.Close();
        break;

    case State::READING:
        m_state = State::SHUTTING_DOWN;
        m_socket.ShutdownBoth();
        // If there is a blocked read on a separate thread, the socket shutdown will unblock it. Wait for it to finish.
        m_readCancel.wait(lock, [this]() { return m_state == State::CLOSED; });
        break;
    }
}

void TcpSocket::Write(void const* src, size_t len)
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    if (m_state == State::CLOSED)
        throw ProgramError("Socket is not connected.");
    if (m_state == State::SHUTTING_DOWN)
        throw ProgramError("Socket was closed from another thread.");

    m_socket.Write(src, len);
}

bool TcpSocket::Read(void* dest, size_t len)
{
    {
        std::lock_guard<std::mutex> lock(m_socketLock);
        if (m_state == State::READING || m_state == State::SHUTTING_DOWN)
            throw ProgramError("Socket is already reading.");
        if (m_state == State::CLOSED)
            throw ProgramError("Socket is not connected.");
        m_state = State::READING;
    }

    try
    {
        bool const stillConnected = m_socket.Read(dest, len);

        std::unique_lock<std::mutex> lock(m_socketLock);
        if (m_state == State::SHUTTING_DOWN)
            throw ProgramError("Socket was closed from another thread.");

        m_state = State::CONNECTED;
        return stillConnected;
    }
    catch (...)
    {
        std::unique_lock<std::mutex> lock(m_socketLock);
        m_state = State::CLOSED;
        m_readCancel.notify_all();
        throw;
    }
}

// returns the amount of bytes available in the stream
// guaranteed not to be bigger than the actual number
// you can read this many bytes without blocking
// may be smaller than the actual number of bytes available
unsigned TcpSocket::DataAvailable()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state == State::CLOSED)
        throw ProgramError("Socket is not connected.");
    if (m_state == State::READING)
        throw ProgramError("Socket is already reading.");
    if (m_state == State::SHUTTING_DOWN)
        throw ProgramError("Socket was closed from another thread.");

    return m_socket.DataAvailable();
}

TcpSocket::operator bool() const
{
    return IsConnected();
}

} }
