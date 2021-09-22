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

#include <strapper/net/TcpSerializer.h>

#include <strapper/net/SocketError.h>
#include <strapper/net/Endian.h>

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
    EndianGloss(&int32);
    m_socket.Write(&int32, sizeof(int32));
}

void TcpSerializer::Write(double d)
{
    EndianGloss(&d);
    m_socket.Write(&d, sizeof(double));
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
        EndianGloss(dest);
    return result;
}

bool TcpSerializer::Read(double* dest)
{
    static_assert(sizeof(unsigned long long) == sizeof(double), "Function not compatible with this compiler.");
    if (!dest)
        throw ProgramError("Null pointer.");

    bool result = m_socket.Read(dest, sizeof(*dest));
    if (result)
        EndianGloss(dest);
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
        throw ProgramError("Received bad string size.");

    dest->resize(static_cast<size_t>(len));
    return m_socket.Read(&*(dest->begin()), static_cast<size_t>(len));
}

} }
