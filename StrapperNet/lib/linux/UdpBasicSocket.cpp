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

#include <strapper/net/UdpBasicSocket.h>

#include <strapper/net/SocketError.h>
#include "SocketFd.h"

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <cassert>
#include <limits>

namespace strapper { namespace net {

namespace {

// Creates a socket and binds it to the given port. Set to 0 for any.
SocketHandle MakeSocket(uint16_t myport)
{
    SocketHandle socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(socket);

    sockaddr_in myInfo{};
    myInfo.sin_family = AF_INET;
    myInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    myInfo.sin_port = htons(myport);

    auto const status = bind(**socket,
                             reinterpret_cast<sockaddr*>(&myInfo),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                             sizeof(myInfo));

    if (status == SocketFd::SOCKET_ERROR)
        throw SocketError(errno);

    return socket;
}

}  // namespace

//! 0 for any. // todo: verify
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

// To keep compatibility with the Windows version, if this timeout is reached,
// the socket will be closed. Thank you, Windows.
// Only use this feature as a robustness mechanism.
// (e.g. so you don't block forever if the connection is somehow silently lost.)
// Don't use this as a form of non-blocking read.
// after setting this, a timed out read will return -1
// you should check errno
//! 0 = no timeout (forever) and is the default setting
void UdpBasicSocket::SetReadTimeout(unsigned milliseconds)
{
    timeval t{};
    t.tv_sec = milliseconds / 1000;                                      // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    t.tv_usec = static_cast<suseconds_t>((milliseconds % 1000) * 1000);  // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    if (setsockopt(**m_socket, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(timeval)) == SocketFd::SOCKET_ERROR)
        throw SocketError(errno);
}

void UdpBasicSocket::Shutdown() noexcept
{
    if (m_socket)
        shutdown(**m_socket, SHUT_RDWR);
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

    sockaddr_in info{};
    info.sin_family = AF_INET;
    info.sin_port = htons(port);
    std::string const ipString = ipAddress.ToString('.');
    int const success = inet_pton(AF_INET, ipString.c_str(), &info.sin_addr);
    if (success == 0)
        throw ProgramError("Invalid IP address: " + ipAddress.ToString('.'));
    if (success == -1)
        throw SocketError(errno);
    if (success != 1)
        throw ProgramError("Unknown error.");

    auto* infoAsSockAddr = reinterpret_cast<sockaddr*>(&info);  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    while (sendto(**m_socket, src, len, 0, infoAsSockAddr, sizeof(info)) == SocketFd::SOCKET_ERROR)
    {
        if (errno != EINTR)
            throw SocketError(errno);
    }
}

// if (amountRead == SOCKET_ERROR)
// {
//     switch (errno)
//     {
//     case EAGAIN:           // Probably means that a timeout was reached. Socket is still good, but for
//         close(lock);       // consistency with the Windows version, we need to close the socket.
//         return false;
//         break;

//     default:
//         assert(false);  // todo: development only. Need to see what kind of errors we experience.
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

    sockaddr_in info{};
    socklen_t infoLen = sizeof(info);
    ssize_t amountRead = 0;
    do
    {
        amountRead = recvfrom(**m_socket, dest, maxlen, 0,
                              reinterpret_cast<sockaddr*>(&info),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                              &infoLen);
    } while (amountRead == SocketFd::SOCKET_ERROR && errno == EINTR);
    if (amountRead == SocketFd::SOCKET_ERROR)
        throw SocketError(errno);
    if (amountRead == 0)
        throw ProgramError("Socket was closed.");

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
    int bytesAvailable = 0;
    int const fd = **m_socket;
    auto const status = ioctl(  // NOLINT(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
        fd,
        FIONREAD,
        &bytesAvailable);
    if (status == SocketFd::SOCKET_ERROR)
        throw SocketError(errno);

    if (bytesAvailable < 0)
        throw ProgramError("ioctl returned invalid value.");

    return static_cast<unsigned>(bytesAvailable);
}

UdpBasicSocket::operator bool() const
{
    return IsOpen();
}

}}  // namespace strapper::net
