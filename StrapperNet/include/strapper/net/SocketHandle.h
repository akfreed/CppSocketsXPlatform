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

#pragma once

#include <memory>

namespace strapper { namespace net {

struct SocketFd;

class SocketHandle
{
public:
    SocketHandle();  // = default
    SocketHandle(int family, int socktype, int protocol);
    explicit SocketHandle(SocketFd const& fd);
    SocketHandle(SocketHandle const&) = delete;
    SocketHandle(SocketHandle&& other) noexcept;  // = default
    SocketHandle& operator=(SocketHandle const&) = delete;
    SocketHandle& operator=(SocketHandle&& other) noexcept;  // = default
    ~SocketHandle();

    void Close() noexcept;

    SocketFd const& Get() const;
    SocketFd const& operator*() const;
    explicit operator bool() const;

private:
    std::unique_ptr<SocketFd> m_socketId;
};

}}  // namespace strapper::net
