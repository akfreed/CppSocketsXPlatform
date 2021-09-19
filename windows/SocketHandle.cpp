// ==================================================================
// Copyright 2021 Alexander K. Freed
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

#include <SocketHandle.h>

#include <SocketError.h>

#include <utility>

namespace strapper { namespace net {

SocketHandle::SocketHandle(int family, int socktype, int protocol)
    : m_socketId(socket(family, socktype, protocol))
{
    if (m_socketId == INVALID_SOCKET)
        throw SocketError(WSAGetLastError());
}

SocketHandle::SocketHandle(SOCKET fd)
    : m_socketId(fd)
{ }

SocketHandle::SocketHandle(SocketHandle&& other) noexcept
    : SocketHandle()
{
    std::swap(m_socketId, other.m_socketId);
}

SocketHandle& SocketHandle::operator=(SocketHandle&& other) noexcept
{
    SocketHandle temp(std::move(other));
    std::swap(m_socketId, temp.m_socketId);
    return *this;
}

SocketHandle::~SocketHandle()
{
    if (*this)
        Close();
}

SocketHandle::operator bool() const
{
    return m_socketId != INVALID_SOCKET;
}

SOCKET const& SocketHandle::Get() const
{
    return m_socketId;
}

void SocketHandle::Close() noexcept
{
    closesocket(m_socketId);
    m_socketId = INVALID_SOCKET;
}

} }
