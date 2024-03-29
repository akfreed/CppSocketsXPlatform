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

#include "SocketIncludes.h"
#include <strapper/net/TcpBasicSocket.h>

#include <strapper/net/SocketError.h>
#include "SocketFd.h"

#include <limits>

namespace strapper { namespace net {

namespace {

//! Connects to host:port
SocketHandle Connect(std::string const& host, uint16_t port)
{
    addrinfo hostInfo{};
    hostInfo.ai_family = AF_UNSPEC;      // Can be IPv4 or IPv6
    hostInfo.ai_socktype = SOCK_STREAM;  // TCP

    auto lFreeList = [](addrinfo* p) { freeaddrinfo(p); };
    std::unique_ptr<addrinfo, decltype(lFreeList)> hostInfoList(nullptr, lFreeList);

    {
        addrinfo* hil = nullptr;
        int const error = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hostInfo, &hil);
        if (error != 0)
            throw SocketError(error);
        if (!hil)
            throw ProgramError("getaddrinfo returned empty list.");
        hostInfoList.reset(hil);
    }

    if (hostInfoList->ai_addrlen > static_cast<size_t>(std::numeric_limits<int>::max()))
        throw ProgramError("getaddrinfo returned invalid length.");

    SocketHandle socket(hostInfoList->ai_family, hostInfoList->ai_socktype, hostInfoList->ai_protocol);

    if (connect(**socket, hostInfoList->ai_addr, static_cast<int>(hostInfoList->ai_addrlen)) == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());

    return socket;
}

}  // namespace

//! Unused for this implementation.
struct TcpBasicSocketImpl
{ };

TcpBasicSocket::TcpBasicSocket() = default;
TcpBasicSocket::TcpBasicSocket(TcpBasicSocket&&) noexcept = default;
TcpBasicSocket& TcpBasicSocket::operator=(TcpBasicSocket&&) noexcept = default;

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

bool TcpBasicSocket::IsOpen() const
{
    return !!m_socket;
}

//! If this timeout is reached, the socket will be closed. Thank you, Windows.
//! Only use this feature as a robustness mechanism.
//! (e.g. so you don't block forever if the connection is somehow silently lost.)
//! Don't use this as a form of non-blocking read.
//! 0 = no timeout (forever) and is the default setting
void TcpBasicSocket::SetReadTimeout(unsigned milliseconds)
{
    DWORD const arg = milliseconds;
    auto const status = setsockopt(**m_socket,
                                   SOL_SOCKET,
                                   SO_RCVTIMEO,
                                   reinterpret_cast<char const*>(&arg),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                                   sizeof(arg));
    if (status == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());
}

void TcpBasicSocket::ShutdownSend()
{
    if (shutdown(**m_socket, SD_SEND) == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());
}

void TcpBasicSocket::ShutdownReceive()
{
    if (shutdown(**m_socket, SD_RECEIVE) == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());
}

void TcpBasicSocket::ShutdownBoth() noexcept
{
    if (m_socket)
    {
        shutdown(**m_socket, SD_BOTH);
        // In winsock, shutdown doesn't cancel a blocking read.
        CancelIoEx(
            reinterpret_cast<HANDLE>(**m_socket),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
            nullptr);
    }
}

//! Shutdown and close the socket.
void TcpBasicSocket::Close() noexcept
{
    ShutdownBoth();
    m_socket.Close();
}

void TcpBasicSocket::Write(void const* src, size_t len)
{
    if (!src)
        throw ProgramError("Null pointer.");
    if (len == 0)
        throw ProgramError("Length must be greater than 0.");
    if (len > static_cast<size_t>(std::numeric_limits<int>::max()))
        throw ProgramError("Length must be less than int max.");

    if (send(**m_socket, static_cast<char const*>(src), static_cast<int>(len), 0) == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());
}

//! Reads len bytes into given buffer.
//! Blocks until all the requested bytes are available.
//! @return True if the read was successful. False if there was an error, in which case the socket is closed.
bool TcpBasicSocket::Read(void* dest, size_t len)
{
    try
    {
        if (!dest)
            throw ProgramError("Null pointer.");
        if (len == 0)
            throw ProgramError("Length must be greater than 0.");
        if (len > static_cast<size_t>(std::numeric_limits<int>::max()))
            throw ProgramError("Length must be less than int max.");

        int const lenAsInt = static_cast<int>(len);
        int const amountRead = recv(**m_socket, static_cast<char*>(dest), lenAsInt, MSG_WAITALL);
        if (amountRead == SOCKET_ERROR)
            throw SocketError(WSAGetLastError());
        if (amountRead == 0 && len > 0)  // Graceful close.
        {
            ShutdownReceive();
            return false;
        }
        if (amountRead != lenAsInt)
            throw ProgramError("Other side closed before all bytes were received.");
        return true;
    }
    catch (...)
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
    u_long bytesAvailable = 0;
    if (ioctlsocket(**m_socket, FIONREAD, &bytesAvailable) == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());

    if (bytesAvailable > std::numeric_limits<unsigned>::max())
        bytesAvailable = std::numeric_limits<unsigned>::max();

    return bytesAvailable;
}

TcpBasicSocket::operator bool() const
{
    return IsOpen();
}

}}  // namespace strapper::net
