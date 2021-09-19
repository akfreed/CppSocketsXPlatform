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

#include <TcpSerializer.h>

#include <SocketError.h>

#include <cassert>
#include <cstring>

namespace strapper { namespace net {

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

void TcpSerializer::Write(char c)
{
    m_socket.Write(&c, 1);
}

void TcpSerializer::Write(bool b)
{
    uint8_t const buf = b ? 1 : 0;
    m_socket.Write(&buf, 1);
}

void TcpSerializer::Write(int32_t int32)
{
    int32 = htonl(int32);  // convert to big endian
    m_socket.Write(&int32, sizeof(int32));
}

void TcpSerializer::Write(double d)
{
    static_assert(sizeof(unsigned long long) == sizeof(double), "Function not compatible with this compiler.");

    uint8_t buffer[sizeof(double)];

    // convert to big endian
    unsigned long long in;
    std::memcpy(&in, &d, sizeof(d));

    for (int i = sizeof(double) - 1; i >= 0; --i)
    {
        buffer[i] = static_cast<uint8_t>(in & 0xFF);
        in >>= 8;
    }

    m_socket.Write(buffer, sizeof(double));
}

void TcpSerializer::Write(std::string const& s)
{
    int const len = (s.length() > MAX_STRING_LEN) ? MAX_STRING_LEN : static_cast<int>(s.length());
    Write(len);
    if (len > 0)
        m_socket.Write(s.c_str(), len);
}

//----------------------------------------------------------------------------

bool TcpSerializer::Read(char* dest)
{
    if (!dest)
        throw ProgramError("Null pointer.");
    return m_socket.Read(dest, 1);
}

bool TcpSerializer::Read(bool* dest)
{
    if (!dest)
        throw ProgramError("Null pointer.");
    uint8_t buf;
    bool result = m_socket.Read(&buf, 1);
    if (result)
        *dest = (buf == 0 ? false : true);
    return result;
}

bool TcpSerializer::Read(int32_t* dest)
{
    if (!dest)
        throw ProgramError("Null pointer.");
    bool result = m_socket.Read(dest, sizeof(*dest));
    if (result)
        *dest = ntohl(*dest);  // convert to host endian
    return result;
}

bool TcpSerializer::Read(double* dest)
{
    static_assert(sizeof(unsigned long long) == sizeof(double), "Function not compatible with this compiler.");
    if (!dest)
        throw ProgramError("Null pointer.");

    uint8_t buffer[sizeof(double)];

    bool result = m_socket.Read(buffer, sizeof(double));
    if (result)
    {
        // convert to host endian
        unsigned long long out = 0;
        for (unsigned i = 0; i < sizeof(double); ++i)
        {
            out <<= 8;
            out |= buffer[i];
        }

        std::memcpy(dest, &out, sizeof(*dest));
    }
    return result;
}

// maxlen is the size of the buffer
// if successful, the string will always be null-terminated
bool TcpSerializer::Read(std::string* dest)
{
    if (!dest)
        throw ProgramError("Null pointer.");

    int len;
    if (!Read(&len))
        return false;

    if (len < 0 || len > MAX_STRING_LEN) // other end is corrupted or is not following the protocol.
        throw SocketError(0);

    dest->resize(static_cast<size_t>(len));
    return m_socket.Read(&*(dest->begin()), static_cast<size_t>(len));
}

} }
