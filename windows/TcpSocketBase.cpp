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

#include <TcpSocketBase.h>

#include <NetworkError.h>

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
        int const error = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hostInfo, &hil);
        if (error != 0 || !hil)
            return {};
        hostInfoList.reset(hil);
    }

    if (hostInfoList->ai_addrlen > std::numeric_limits<int>::max())
        return {};

    SocketHandle socket(hostInfoList->ai_family, hostInfoList->ai_socktype, hostInfoList->ai_protocol);
    if (!socket)
        return {};

    int const error = connect(socket.Get(), hostInfoList->ai_addr, static_cast<int>(hostInfoList->ai_addrlen));
    if (error == SOCKET_ERROR)
        return {};

    return socket;
}

}

//! Constructor connects to host:port.
TcpSocketBase::TcpSocketBase(std::string const& host, uint16_t port)
    : m_socket(Connect(host, port))
{ }

//! Special private constructor used only by TcpListener.Accept().
TcpSocketBase::TcpSocketBase(SocketHandle&& socket)
    : m_socket(std::move(socket))
{ }

TcpSocketBase::~TcpSocketBase()
{
    if (*this)
        Close();
}

bool TcpSocketBase::IsConnected() const
{
    return !!m_socket;
}

//! If this timeout is reached, the socket will be closed. Thank you, Windows.
//! Only use this feature as a robustness mechanism.
//! (e.g. so you don't block forever if the connection is somehow silently lost.)
//! Don't use this as a form of non-blocking read.
void TcpSocketBase::SetReadTimeout(unsigned milliseconds)
{
    DWORD const arg = milliseconds;
    if (setsockopt(m_socket.Get(), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char const*>(&arg), sizeof(arg)) == SOCKET_ERROR)
        ErrorCode(WSAGetLastError()).ThrowIfError();
}

ErrorCode TcpSocketBase::ShutdownSend() noexcept
{
    return (shutdown(m_socket.Get(), SD_SEND) == SOCKET_ERROR) ? ErrorCode(WSAGetLastError()) : ErrorCode();
}

ErrorCode TcpSocketBase::ShutdownReceive() noexcept
{
    return (shutdown(m_socket.Get(), SD_RECEIVE) == SOCKET_ERROR) ? ErrorCode(WSAGetLastError()) : ErrorCode();
}

ErrorCode TcpSocketBase::ShutdownBoth() noexcept
{
    return (shutdown(m_socket.Get(), SD_BOTH) == SOCKET_ERROR) ? ErrorCode(WSAGetLastError()) : ErrorCode();
}

//! Shutdown and close the socket.
ErrorCode TcpSocketBase::Close() noexcept
{
    ShutdownBoth();
    return m_socket.Close();
}

void TcpSocketBase::Write(void const* src, size_t len)
{
    if (len > std::numeric_limits<int>::max())
        throw NetworkProgrammingError("Length must be less than int max.");

    if (send(m_socket.Get(), reinterpret_cast<char const*>(src), static_cast<int>(len), 0) == SOCKET_ERROR)
        ErrorCode(WSAGetLastError()).ThrowIfError();
}

//! Reads len bytes into given buffer.
//! Blocks until all the requested bytes are available.
//! @return True if the read was successful. False if there was an error, in which case the socket is closed.
bool TcpSocketBase::Read(void* dest, size_t len)
{
    try
    {
        if (len > std::numeric_limits<int>::max())
            throw NetworkProgrammingError("Length must be less than int max.");

        int const lenAsInt = static_cast<int>(len);
        int amountRead = recv(m_socket.Get(), reinterpret_cast<char*>(dest), lenAsInt, MSG_WAITALL);
        if (amountRead == SOCKET_ERROR)
            ErrorCode(WSAGetLastError()).ThrowIfError();
        if (amountRead == 0 && len > 0)
        {
            ShutdownReceive().ThrowIfError();
            return false;
        }
        if (amountRead != lenAsInt)
            throw NetworkConnectionError("Other side closed before all bytes were received.");
        return true;
    }
    catch (NetworkError const&)
    {
        Close();
        throw;
    }
}

//! Returns the amount of bytes available in the stream.
//! Guaranteed not to be bigger than the actual number.
//! You can read this many bytes without blocking.
//! May be smaller than the actual number of bytes available
unsigned TcpSocketBase::DataAvailable()
{
    unsigned long bytesAvailable = 0;
    if (ioctlsocket(m_socket.Get(), FIONREAD, &bytesAvailable) == SOCKET_ERROR)
        ErrorCode(WSAGetLastError()).ThrowIfError();

    if (bytesAvailable > std::numeric_limits<unsigned>::max())
        bytesAvailable = std::numeric_limits<unsigned>::max();

    return bytesAvailable;
}

TcpSocketBase::operator bool() const
{
    return IsConnected();
}

} }
