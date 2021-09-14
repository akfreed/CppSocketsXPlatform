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

#pragma once

#include <SocketIncludes.h>
#include <ErrorCode.h>

class SocketHandle
{
public:
    SocketHandle() = default;
    SocketHandle(int family, int socktype, int protocol);
    explicit SocketHandle(SOCKET fd);
    SocketHandle(SocketHandle const&) = delete;
    SocketHandle(SocketHandle&& other) noexcept;
    SocketHandle& operator=(SocketHandle const&) = delete;
    SocketHandle& operator=(SocketHandle&& other) noexcept;
    ~SocketHandle();

    explicit operator bool() const;
    SOCKET const& Get() const;
    ErrorCode Close();

private:
    SOCKET m_socketId = INVALID_SOCKET;
};