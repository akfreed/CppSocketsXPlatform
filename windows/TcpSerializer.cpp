// ==================================================================
// Copyright 2018-2021 Alexander K. Freed
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

// Contains the declaration TcpSocket
//
// This file should be included before windows.h
//
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.

#include "TcpSerializer.h"

#include <cassert>
#include <stdexcept>

TcpSerializer::TcpSerializer(TcpSocket&& socket)
    : m_socket(std::move(socket))
{ }

const TcpSocket& TcpSerializer::Socket() const
{
    return m_socket;
}

TcpSocket& TcpSerializer::Socket()
{
    return m_socket;
}

bool TcpSerializer::Write(char c)
{
    return m_socket.Write(&c, 1);
}

bool TcpSerializer::Write(bool b)
{
    const char c = static_cast<char>(b);
    return m_socket.Write(&c, 1);
}

bool TcpSerializer::Write(int32_t int32)
{
    static_assert(sizeof(int32_t) == 4, "Function not compatible with this architecture.");
    static_assert(sizeof(char) == 1, "Function not compatible with this architecture.");

    char buffer[sizeof(int32_t)];

    int32 = htonl(int32);  // convert to big endian
    memcpy(buffer, &int32, sizeof(int32));

    return m_socket.Write(buffer, 4);
}

bool TcpSerializer::Write(double d)
{
    static_assert(sizeof(unsigned long long) == 8, "Function not compatible with this architecture.");
    static_assert(sizeof(double) == 8, "Function not compatible with this architecture.");
    static_assert(sizeof(char) == 1, "Function not compatible with this architecture.");

    char buffer[sizeof(double)];

    // convert to big endian
    unsigned long long in;
    memcpy(&in, &d, sizeof(d));

    for (int i = sizeof(double) - 1; i >= 0; --i)
    {
        buffer[i] = static_cast<char>(in & 0xFF);
        in >>= 8;
    }

    return m_socket.Write(buffer, 8);
}

bool TcpSerializer::WriteString(const char* str)
{
    int len = static_cast<int>(strlen(str)); // todo: Use size_t.
    if (len > MAX_STRING_LEN)
        len = MAX_STRING_LEN;

    return Write(len) && m_socket.Write(str, len);
}

//----------------------------------------------------------------------------

bool TcpSerializer::Read(char& dest)
{
    return m_socket.Read(&dest, 1);
}

bool TcpSerializer::Read(bool& dest)
{
    char buf;
    bool result = m_socket.Read(&buf, 1);
    if (result)
        dest = static_cast<bool>(buf);
    return result;
}

bool TcpSerializer::Read(int32_t& dest)
{
    static_assert(sizeof(int32_t) == 4, "Function not compatible with this architecture.");
    static_assert(sizeof(char) == 1, "Function not compatible with this architecture.");

    char buffer[sizeof(int32_t)];

    bool result = m_socket.Read(buffer, 4);

    if (result)
    {
        memcpy(&dest, buffer, sizeof(dest));
        dest = ntohl(dest);  // convert to host endian
    }
    return result;
}

bool TcpSerializer::Read(double& dest)
{
    static_assert(sizeof(unsigned long long) == 8, "Function not compatible with this architecture.");
    static_assert(sizeof(double) == 8, "Function not compatible with this architecture.");
    static_assert(sizeof(char) == 1, "Function not compatible with this architecture.");

    char buffer[sizeof(double)];

    bool result = m_socket.Read(buffer, 8);

    if (result)
    {
        // convert to host endian
        unsigned long long out = 0;
        for (unsigned i = 0; i < sizeof(double); ++i)
        {
            out <<= 8;
            out |= buffer[i] & 0xFF;
        }

        memcpy(&dest, &out, sizeof(dest));
    }
    return result;
}

// maxlen is the size of the buffer
// if successful, the string will always be null-terminated
int TcpSerializer::ReadString(char* c, int maxlen)
{
    if (maxlen < 1)
    {
        throw std::runtime_error("Max length must be greater than 0.");
    }

    int len;
    if (!Read(len))
        return -1;

    if (len < 1 || len > maxlen)
    {
        // other end is corrupted or is not following the protocol.
        assert(false);  // todo: development only. Please remove from final version
        m_socket.Close();
        return -1;
    }

    if (!m_socket.Read(c, len))
        return -1;

    if (len < maxlen)
        c[len++] = '\0';
    else
        c[len - 1] = '\0';

    return len;
}
