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

#include "SocketIncludes.h"
#include <strapper/net/TcpBasicListener.h>

#include <strapper/net/SocketError.h>
#include "SocketFd.h"

#include <cassert>
#include <memory>

namespace strapper { namespace net {

namespace {

SocketHandle Start(uint16_t port)
{
    addrinfo hostInfo{};
    hostInfo.ai_family = AF_INET;
    hostInfo.ai_socktype = SOCK_STREAM;
    hostInfo.ai_protocol = IPPROTO_TCP;
    hostInfo.ai_flags = AI_PASSIVE;

    auto lFreeList = [](addrinfo* p) { freeaddrinfo(p); };
    std::unique_ptr<addrinfo, decltype(lFreeList)> hostInfoList(nullptr, lFreeList);

    {
        addrinfo* hil = nullptr;
        int const error = getaddrinfo(nullptr, std::to_string(port).c_str(), &hostInfo, &hil);
        if (error != 0)
            throw SocketError(error);
        if (!hil)
            throw SocketError(0);
        hostInfoList.reset(hil);
    }

    SocketHandle sock = SocketHandle(hostInfoList->ai_family, hostInfoList->ai_socktype, hostInfoList->ai_protocol);
    assert(sock);

    /*
    BOOL const yes = true;
    if (setsockopt(sock.Get(), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char const*>(&yes), sizeof(yes)) == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());
    // */

    if (bind(**sock, hostInfoList->ai_addr, static_cast<int>(hostInfoList->ai_addrlen)) == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());

    if (listen(**sock, 128) == SOCKET_ERROR)
        throw SocketError(WSAGetLastError());

    return sock;
}

}

TcpBasicListener::TcpBasicListener(uint16_t port)
    : m_socket(Start(port))
{ }

TcpBasicListener::~TcpBasicListener()
{
    if (*this)
        Close();
}

bool TcpBasicListener::IsListening() const
{
    return !!m_socket;
}

void TcpBasicListener::Close() noexcept
{
    shutdown();
    m_socket.Close();
}

TcpBasicSocket TcpBasicListener::Accept()
{
    try
    {
        for (int retries = 0; ; ++retries)
        {
            SOCKET clientId = accept(**m_socket, nullptr, nullptr);  // todo: implement args 2 and 3
            if (clientId == INVALID_SOCKET)
            {
                int const error = WSAGetLastError();
                if (error == WSAECONNRESET && retries < 10)
                    continue;
                throw SocketError(error);
            }
            return TcpBasicSocket::Attorney::accept(SocketHandle(SocketFd{ clientId }));
        }
    }
    catch (ProgramError const&)
    {
        Close();
        throw;
    }
}

TcpBasicListener::operator bool() const
{
    return IsListening();
}

void TcpBasicListener::shutdown() noexcept
{
    ::shutdown(**m_socket, SD_BOTH);
}

} }
