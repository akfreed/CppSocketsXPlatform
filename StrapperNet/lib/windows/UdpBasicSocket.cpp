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
#include <strapper/net/UdpBasicSocket.h>

#include <strapper/net/SocketError.h>
#include "SocketFd.h"

#include <limits>

namespace strapper { namespace net {

namespace {

// Creates a socket and binds it to the given port. Set to 0 for any.
SocketHandle MakeSocket(uint16_t myport)
{
    SocketHandle socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in myInfo{};
    myInfo.sin_family = AF_INET;
    myInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    myInfo.sin_port = htons(myport);

    auto const status = bind(**socket,
                             reinterpret_cast<sockaddr*>(&myInfo),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                             sizeof(myInfo));
    if (status == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());

    return socket;
}

}  // namespace

//! 0 for any.
UdpBasicSocket::UdpBasicSocket(uint16_t myport)
    : m_socket(MakeSocket(myport))
{ }

UdpBasicSocket::~UdpBasicSocket()
{
    if (*this)
        Close();
}

bool UdpBasicSocket::IsOpen() const
{
    return !!m_socket;
}

// If this timeout is reached, the socket will be closed. Thank you, Windows.
// Only use this feature as a robustness mechanism.
// (e.g. so you don't block forever if the connection is somehow silently lost.)
// Don't use this as a form of non-blocking read.
void UdpBasicSocket::SetReadTimeout(unsigned milliseconds)
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

void UdpBasicSocket::Shutdown() noexcept
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

// Shutdown and close the socket.
void UdpBasicSocket::Close() noexcept
{
    Shutdown();
    m_socket.Close();
}

void UdpBasicSocket::Write(void const* src, size_t len, IpAddressV4 const& ipAddress, uint16_t port)
{
    if (!src)
        throw ProgramError("Null pointer.");
    if (len == 0)
        throw ProgramError("Length must be greater than 0.");
    if (len > static_cast<size_t>(std::numeric_limits<int>::max()))
        throw ProgramError("Length must be less than int max.");

    sockaddr_in info{};
    info.sin_family = AF_INET;
    info.sin_port = htons(port);
    std::string const ipString = ipAddress.ToString('.');
    int const success = inet_pton(AF_INET, ipString.c_str(), &info.sin_addr);
    if (success == 0)
        throw ProgramError("Invalid IP address: " + ipAddress.ToString('.'));
    if (success == -1)
        throw SocketError(WSAGetLastError());
    if (success != 1)
        throw ProgramError("Unknown error.");

    int const amountWritten = sendto(**m_socket,
                                     static_cast<char const*>(src),
                                     static_cast<int>(len),
                                     0,
                                     reinterpret_cast<sockaddr*>(&info),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                                     sizeof(info));
    if (amountWritten == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());
}

// if (amountRead == SOCKET_ERROR)
// {
//     int error = WSAGetLastError();
//     switch (error)
//     {
//     case WSAEINTR:       // blocking call was interrupted. In a multi-threaded environment, probably means the socket was closed by another thread
//         assert(false);   // This shouldn't happen since this situation should be protected by the state machine and mutexes.
//         return false;
//         break;

//     case WSAETIMEDOUT:   // timeout was reached. If this happens, the socket is in an invalid state and must be closed. (Thanks, Windows)
//         m_state = State::OPEN;
//         close(lock);
//         return false;
//         break;

//     case WSAEMSGSIZE:    // buffer was not large enough
//         amountRead = maxlen;
//         break;  // fall out of error check back to normal return

//     case WSAECONNRESET:  // In TCP, this is a hard reset. In UDP, it means a previous write failed (ICMP Port Unreachable).
//         if (!m_theirInfoIsValid)  // expected behavior when reusing a socket. However, we don't allow socket reuse in this implementation.
//         {
//             assert(false);
//             return false;
//         }
//         else if (checkSenderInfo(info, infoLen, lock))  // same sender. Up to calling code if they want to close
//         {
//             return false;
//         }
//         // else, different sender. fall out of error check back to normal return, which will try the read again
//         break;

//     default:
//         assert(false);  // todo: development only. Need to see what kind of errors we experience.
//         m_state = State::OPEN;
//         close(lock);
//         return false;
//         break;
//     }
// }

unsigned UdpBasicSocket::Read(void* dest, size_t maxlen, IpAddressV4* out_ipAddress, uint16_t* out_port)
{
    if (!dest)
        throw ProgramError("Null pointer.");
    if (maxlen == 0)
        throw ProgramError("Max length must be greater than 0.");
    if (maxlen > static_cast<size_t>(std::numeric_limits<int>::max()))
        throw ProgramError("Max length must be less than int max.");

    sockaddr_in info{};
    int infoLen = sizeof(info);
    int const amountRead = recvfrom(**m_socket,
                                    static_cast<char*>(dest),
                                    static_cast<int>(maxlen),
                                    0,
                                    reinterpret_cast<sockaddr*>(&info),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                                    &infoLen);
    if (amountRead == 0)
        throw ProgramError("Socket was shut down.");
    if (amountRead == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());

    if ((out_ipAddress || out_port) && (infoLen != sizeof(info)))
        throw ProgramError("Read returned unexpected endpoint info size.");

    if (out_ipAddress)
        *out_ipAddress = IpAddressV4(info.sin_addr.s_addr);
    if (out_port)
        *out_port = ntohs(info.sin_port);

    return static_cast<unsigned>(amountRead);
}

// returns the total amount of data in the buffer.
// A call to Read will not necessarily return this much data, since the buffer may contain many datagrams.
// returns -1 on error
unsigned UdpBasicSocket::DataAvailable() const
{
    u_long bytesAvailable = 0;
    if (ioctlsocket(**m_socket, FIONREAD, &bytesAvailable) == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());

    if (bytesAvailable > std::numeric_limits<unsigned>::max())
        bytesAvailable = std::numeric_limits<unsigned>::max();

    return bytesAvailable;
}

UdpBasicSocket::operator bool() const
{
    return IsOpen();
}

}}  // namespace strapper::net
