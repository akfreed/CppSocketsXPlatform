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

#include <strapper/net/TcpBasicSocket.h>

#include <strapper/net/SocketError.h>
#include "SocketFd.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>

#include <cassert>
#include <memory>
#include <limits>

namespace strapper { namespace net {

namespace {

//! Connects to host:port
SocketHandle Connect(std::string const& host, uint16_t port)
{
    addrinfo hostInfo{};
    hostInfo.ai_family = AF_UNSPEC; // Can be IPv4 or IPv6
    hostInfo.ai_socktype = SOCK_STREAM; // TCP

    auto lFreeList = [](addrinfo* p) { freeaddrinfo(p); };
    std::unique_ptr<addrinfo, decltype(lFreeList)> hostInfoList(nullptr, lFreeList);

    {
        addrinfo* hil = nullptr;
        std::string const portString = std::to_string(port);
        int const error = getaddrinfo(host.c_str(), portString.c_str(), &hostInfo, &hil);
        if (error != 0)
            throw SocketError(error);
        if (!hil)
            throw ProgramError("getaddrinfo returned empty list.");
        hostInfoList.reset(hil);
    }

    SocketHandle socket(hostInfoList->ai_family, hostInfoList->ai_socktype, hostInfoList->ai_protocol);
    assert(socket);

    while (connect(**socket, hostInfoList->ai_addr, hostInfoList->ai_addrlen) == SocketFd::SOCKET_ERROR)
    {
        if (errno != EINTR)
            throw SocketError(errno);
    }

    return socket;
}

}

//! Constructor connects to host:port.
TcpBasicSocket::TcpBasicSocket(std::string const& host, uint16_t port)
    : m_socket(Connect(host, port))
{ }

//! Special private constructor used only by TcpListener.Accept().
TcpBasicSocket::TcpBasicSocket(SocketHandle&& socket)
    : m_socket(std::move(socket))
{ }

TcpBasicSocket::~TcpBasicSocket()
{
    if (*this)
        Close();
}

bool TcpBasicSocket::IsConnected() const
{
    return !!m_socket;
}

// To keep compatibility with the Windows version, if this timeout is reached,
// the socket will be closed. Thank you, Windows.
// Only use this feature as a robustness mechanism.
// (e.g. so you don't block forever if the connection is somehow silently lost.)
// Don't use this as a form of non-blocking read.
// after setting this, a timed out read will return -1
// you should check errno
//! 0 = no timeout (forever) and is the default setting
void TcpBasicSocket::SetReadTimeout(unsigned milliseconds)
{
    timeval t;
    t.tv_sec = milliseconds / 1000;
    t.tv_usec = (milliseconds % 1000) * 1000;
    if (setsockopt(**m_socket, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(timeval)) == SocketFd::SOCKET_ERROR)
        throw SocketError(errno);
}

void TcpBasicSocket::ShutdownSend()
{
    if (shutdown(**m_socket, SHUT_WR) == SocketFd::SOCKET_ERROR)
        throw SocketError(errno);
}

void TcpBasicSocket::ShutdownReceive()
{
    if (shutdown(**m_socket, SHUT_RD) == SocketFd::SOCKET_ERROR)
        throw SocketError(errno);
}

void TcpBasicSocket::ShutdownBoth() noexcept
{
    if (m_socket)
        shutdown(**m_socket, SHUT_RDWR);
}

//! Shutdown and close the socket.
void TcpBasicSocket::Close() noexcept
{
    ShutdownBoth();
    m_socket.Close();
}

void TcpBasicSocket::Write(void const* src, size_t len)
{
    if (len == 0)
        throw ProgramError("Length must be greater than 0.");

    while (send(**m_socket, src, len, 0) == SocketFd::SOCKET_ERROR)
    {
        if (errno != EINTR)
            throw SocketError(errno);
    }
}

//! Reads len bytes into given buffer.
//! Blocks until all the requested bytes are available.
//! @return True if the read was successful. False if there was an error, in which case the socket is closed.
bool TcpBasicSocket::Read(void* dest, size_t len)
{
    try
    {
        if (len == 0)
            throw ProgramError("Length must be greater than 0.");
        if (len > static_cast<size_t>(std::numeric_limits<ssize_t>::max()))
            throw ProgramError("Length must be less than ssize_t max.");
        ssize_t const lenAsLongInt = static_cast<ssize_t>(len);

        ssize_t amountRead = 0;
        do
        {
            amountRead = recv(**m_socket, dest, len, MSG_WAITALL);
        } while (amountRead == SocketFd::SOCKET_ERROR && errno == EINTR);
        if (amountRead == SocketFd::SOCKET_ERROR)
            throw SocketError(errno);
        if (amountRead == lenAsLongInt)
            return true;
        if (amountRead == 0) // Graceful close.
        {
            ShutdownReceive();
            return false;
        }
        throw ProgramError("Other side closed before all bytes were received.");
    }
    catch (ProgramError const&)
    {
        Close();
        throw;
    }
}

//! Returns the amount of bytes available in the stream.
//! Guaranteed not to be bigger than the actual number.
//! You can read this many bytes without blocking.
//! May be smaller than the actual number of bytes available
unsigned TcpBasicSocket::DataAvailable()
{
    int bytesAvailable = 0;
    if (ioctl(**m_socket, FIONREAD, &bytesAvailable) == SocketFd::SOCKET_ERROR)
        throw SocketError(errno);

    if (bytesAvailable < 0)
        throw ProgramError("ioctl returned invalid value.");

    return static_cast<unsigned>(bytesAvailable);
}

TcpBasicSocket::operator bool() const
{
    return IsConnected();
}

} }
