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

namespace strapper { namespace net {

struct SocketFd
{
    static int constexpr INVALID_SOCKET = -1;
    static int constexpr SOCKET_ERROR = -1;

    explicit SocketFd(int fd)
        : m_fd(fd)
    { }

    int& operator*() { return m_fd; }
    int const& operator*() const { return m_fd; }

    int m_fd = INVALID_SOCKET;
};

}} // namespace strapper::net
