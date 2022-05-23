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

#pragma once

#include <strapper/net/TcpSocket.h>

#include <cstdint>
#include <string>

namespace strapper { namespace net {

class TcpSerializer
{
public:
    static constexpr int c_maxStringLen = 1024 * 1024;

    explicit TcpSerializer(TcpSocket&& socket);

    TcpSocket const& Socket() const;
    TcpSocket& Socket();

    void Write(char c, ErrorCode* ec = nullptr);
    void Write(bool b, ErrorCode* ec = nullptr);
    void Write(int32_t int32, ErrorCode* ec = nullptr);
    void Write(double d, ErrorCode* ec = nullptr);
    void Write(std::string const& s, ErrorCode* ec = nullptr);

    bool Read(char* dest, ErrorCode* ec = nullptr);
    bool Read(bool* dest, ErrorCode* ec = nullptr);
    bool Read(int32_t* dest, ErrorCode* ec = nullptr);
    bool Read(double* dest, ErrorCode* ec = nullptr);
    bool Read(std::string* dest, ErrorCode* ec = nullptr);

private:
    TcpSocket m_socket;
};

}}  // namespace strapper::net
