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

#pragma once

#include <TcpSocket.h>

namespace strapper { namespace net {

class TcpSerializer
{
public:
    static constexpr int MAX_STRING_LEN = 4096;

    explicit TcpSerializer(TcpSocket&& socket);

    const TcpSocket& Socket() const;
    TcpSocket& Socket();

    void Write(char c);
    void Write(bool b);
    void Write(int32_t int32);
    void Write(double d);
    void WriteString(const char* str);

    bool Read(char& dest);
    bool Read(bool& dest);
    bool Read(int32_t& dest);
    bool Read(double& dest);
    int ReadString(char* c, int maxlen);

private:
    TcpSocket m_socket;
};

} }
