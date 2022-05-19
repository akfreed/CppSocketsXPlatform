// ==================================================================
// Copyright 2021-2022 Alexander K. Freed
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
#include <strapper/net/SocketHandle.h>

#include <strapper/net/SocketError.h>
#include "SocketFd.h"

namespace strapper { namespace net {

SocketHandle::SocketHandle() = default;
SocketHandle::SocketHandle(SocketHandle&& other) noexcept = default;
SocketHandle& SocketHandle::operator=(SocketHandle&& other) noexcept = default;

SocketHandle::SocketHandle(int family, int socktype, int protocol)
    : m_socketId(new SocketFd{ socket(family, socktype, protocol) })
{
    if (m_socketId->m_fd == INVALID_SOCKET)
        throw SocketError(WSAGetLastError());
}

SocketHandle::SocketHandle(SocketFd const& fd)
    : m_socketId(new SocketFd{ fd })
{ }

SocketHandle::~SocketHandle()
{
    if (*this)
        Close();
}

void SocketHandle::Close() noexcept
{
    if (m_socketId)
    {
        closesocket(m_socketId->m_fd);
        m_socketId.reset();
    }
}

SocketFd const& SocketHandle::Get() const
{
    return *m_socketId;
}

SocketFd const& SocketHandle::operator*() const
{
    return *m_socketId;
}

SocketHandle::operator bool() const
{
    return m_socketId && m_socketId->m_fd != INVALID_SOCKET;
}

}}  // namespace strapper::net
