// ==================================================================
// Copyright 2018-2022 Alexander K. Freed
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

#include <strapper/net/UdpSocket.h>

#include <strapper/net/SocketError.h>

#include <cassert>

namespace strapper { namespace net {

UdpSocket::UdpSocket(uint16_t myport, ErrorCode* ec /* = nullptr */)
{
    try
    {
        m_socket = UdpBasicSocket(myport);
        m_state = State::OPEN;
    }
    catch (ProgramError const&)
    {
        if (!ec)
            throw;
        *ec = ErrorCode(std::current_exception());
    }
}

UdpSocket::UdpSocket(UdpSocket&& other) noexcept
    : UdpSocket()
{
    swap(*this, other);
}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept
{
    UdpSocket temp(std::move(other));
    swap(*this, temp);
    return *this;
}

UdpSocket::~UdpSocket()
{
    Close();
}

void swap(UdpSocket& left, UdpSocket& right)
{
    using State = UdpSocket::State;
    std::lock(left.m_socketLock, right.m_socketLock);  // deadlock-proof
    std::unique_lock<std::mutex> leftLock(left.m_socketLock, std::adopt_lock);
    std::unique_lock<std::mutex> rightLock(right.m_socketLock, std::adopt_lock);

    // If the other socket is reading, moving it probably create a logic bug.
    assert(left.m_state != State::READING);
    assert(right.m_state != State::READING);
    // If the other socket is shutting down, wait for it to finish.
    if (left.m_state == State::SHUTTING_DOWN)
        left.m_readCancel.wait(leftLock, [&left]() { return left.m_state == State::CLOSED; });
    if (right.m_state == State::SHUTTING_DOWN)
        right.m_readCancel.wait(rightLock, [&right]() { return right.m_state == State::CLOSED; });

    using std::swap;
    swap(left.m_socket, right.m_socket);
    swap(left.m_state, right.m_state);
}

bool UdpSocket::IsOpen() const
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    return m_state != State::CLOSED;
}

// If this timeout is reached, the socket will be closed. Thank you, Windows.
// Only use this feature as a robustness mechanism.
// (e.g. so you don't block forever if the connection is somehow silently lost.)
// Don't use this as a form of non-blocking read.
void UdpSocket::SetReadTimeout(unsigned milliseconds, ErrorCode* ec /* = nullptr */)
{
    try
    {
        std::lock_guard<std::mutex> lock(m_socketLock);
        if (m_state == State::CLOSED)
            throw ProgramError("Socket is not open.");
        if (m_state == State::READING)
            throw ProgramError("Socket is already reading.");
        if (m_state == State::SHUTTING_DOWN)
            throw ProgramError("Socket was closed from another thread.");

        m_socket.SetReadTimeout(milliseconds);
    }
    catch (ProgramError const&)
    {
        if (!ec)
            throw;
        *ec = ErrorCode(std::current_exception());
    }
}

// Shutdown and close the socket.
void UdpSocket::Close() noexcept
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    switch (m_state)
    {
    case State::CLOSED:
        break;

    case State::SHUTTING_DOWN:
        m_readCancel.wait(lock, [this]() { return m_state == State::CLOSED; });
        break;

    case State::OPEN:
        m_state = State::CLOSED;
        m_socket.Close();
        break;

    case State::READING:
        m_state = State::SHUTTING_DOWN;
        m_socket.Shutdown();
        // If there is a blocked read on a separate thread, the socket shutdown will unblock it. Wait for it to finish.
        m_readCancel.wait(lock, [this]() { return m_state == State::CLOSED; });
        break;
    }
}

void UdpSocket::Write(void const* src, size_t len, IpAddressV4 const& ipAddress, uint16_t port, ErrorCode* ec /* = nullptr */)
{
    try
    {
        std::unique_lock<std::mutex> lock(m_socketLock);
        if (m_state == State::CLOSED)
            throw ProgramError("Socket is not open.");
        if (m_state == State::SHUTTING_DOWN)
            throw ProgramError("Socket was closed from another thread.");

        return m_socket.Write(src, len, ipAddress, port);
    }
    catch (ProgramError const&)
    {
        if (!ec)
            throw;
        *ec = ErrorCode(std::current_exception());
    }
}

unsigned UdpSocket::Read(void* dest, size_t maxlen, IpAddressV4* out_ipAddress, uint16_t* out_port, ErrorCode* ec /* = nullptr */)
{
    try
    {
        return read(dest, maxlen, out_ipAddress, out_port);
    }
    catch (ProgramError const&)
    {
        if (!ec)
            throw;
        *ec = ErrorCode(std::current_exception());
        return 0;
    }
}

// returns the total amount of data in the buffer.
// A call to Read will not necessarily return this much data, since the buffer may contain many datagrams
unsigned UdpSocket::DataAvailable(ErrorCode* ec /* = nullptr */) const
{
    try
    {
        std::lock_guard<std::mutex> lock(m_socketLock);
        if (m_state == State::CLOSED)
            throw ProgramError("Socket is not open.");
        if (m_state == State::READING)
            throw ProgramError("Socket is already reading.");
        if (m_state == State::SHUTTING_DOWN)
            throw ProgramError("Socket was closed from another thread.");

        return m_socket.DataAvailable();
    }
    catch (ProgramError const&)
    {
        if (!ec)
            throw;
        *ec = ErrorCode(std::current_exception());
        return 0;
    }
}

UdpSocket::operator bool() const
{
    return IsOpen();
}

unsigned UdpSocket::read(void* dest, size_t maxlen, IpAddressV4* out_ipAddress, uint16_t* out_port)
{
    {
        std::lock_guard<std::mutex> lock(m_socketLock);
        if (m_state == State::READING || m_state == State::SHUTTING_DOWN)
            throw ProgramError("Socket is already reading.");
        if (m_state == State::CLOSED)
            throw ProgramError("Socket is not open.");
        m_state = State::READING;
    }

    try
    {
        auto amountRead = m_socket.Read(dest, maxlen, out_ipAddress, out_port);

        std::unique_lock<std::mutex> lock(m_socketLock);
        if (m_state == State::SHUTTING_DOWN)
            throw ProgramError("Socket was closed from another thread.");

        m_state = State::OPEN;
        return amountRead;
    }
    catch (...)
    {
        std::unique_lock<std::mutex> lock(m_socketLock);
        if (m_state == State::SHUTTING_DOWN)
        {
            m_state = State::CLOSED;
            m_socket.Close();
            m_readCancel.notify_all();
        }
        else
            m_state = State::OPEN;
        throw;
    }
}

}}  // namespace strapper::net
