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

#include <strapper/net/TcpSerializer.h>

#include <strapper/net/Endian.h>
#include <strapper/net/SocketError.h>

#include <algorithm>
#include <cstring>

namespace strapper { namespace net {

TcpSerializer::TcpSerializer(TcpSocket&& socket)
    : m_socket(std::move(socket))
{ }

TcpSocket const& TcpSerializer::Socket() const
{
    return m_socket;
}

TcpSocket& TcpSerializer::Socket()
{
    return m_socket;
}

void TcpSerializer::Write(char c, ErrorCode* ec /* = nullptr */)
{
    m_socket.Write(&c, 1, ec);
}

void TcpSerializer::Write(bool b, ErrorCode* ec /* = nullptr */)
{
    uint8_t const buf = b ? 1 : 0;
    m_socket.Write(&buf, 1, ec);
}

void TcpSerializer::Write(int32_t int32, ErrorCode* ec /* = nullptr */)
{
    nton(&int32);
    m_socket.Write(&int32, sizeof(int32), ec);
}

void TcpSerializer::Write(double d, ErrorCode* ec /* = nullptr */)
{
    nton(&d);
    m_socket.Write(&d, sizeof(double), ec);
}

void TcpSerializer::Write(std::string const& s, ErrorCode* ec /* = nullptr */)
{
    try
    {
        static_assert(c_maxStringLen >= 0, "Max string length cannot be less than 0.");
        if (static_cast<size_t>(c_maxStringLen) > s.max_size())
            throw ProgramError("Max string length exceeds std::string max size.");
        if (s.length() > static_cast<size_t>(c_maxStringLen))
            throw ProgramError("String length exceeds max allowed.");

        int const len = static_cast<int>(s.length());

        Write(len);
        if (len > 0)
            m_socket.Write(s.c_str(), static_cast<size_t>(len));
    }
    catch (ProgramError const&)
    {
        if (!ec)
            throw;
        *ec = ErrorCode(std::current_exception());
    }
}

//----------------------------------------------------------------------------

bool TcpSerializer::Read(char* dest, ErrorCode* ec /* = nullptr */)
{
    return m_socket.Read(dest, 1, ec);
}

bool TcpSerializer::Read(bool* dest, ErrorCode* ec /* = nullptr */)
{
    uint8_t buf = 0;
    if (!m_socket.Read(&buf, 1, ec))
        return false;

    *dest = (buf == 0 ? false : true);  // NOLINT(readability-simplify-boolean-expr): This is more readable.
    return true;
}

bool TcpSerializer::Read(int32_t* dest, ErrorCode* ec /* = nullptr */)
{
    if (!m_socket.Read(dest, sizeof(*dest), ec))
        return false;

    nton(dest);
    return true;
}

bool TcpSerializer::Read(double* dest, ErrorCode* ec /* = nullptr */)
{
    static_assert(sizeof(double) == sizeof(uint64_t), "This function is designed for 64-bit doubles.");

    if (!m_socket.Read(dest, sizeof(*dest), ec))
        return false;

    nton(dest);
    return true;
}

// maxlen is the size of the buffer
// if successful, the string will always be null-terminated
bool TcpSerializer::Read(std::string* dest, ErrorCode* ec /* = nullptr */)
{
    try
    {
        static_assert(c_maxStringLen >= 0, "Max string length cannot be less than 0.");
        if (static_cast<size_t>(c_maxStringLen) > dest->max_size())
            throw ProgramError("Max string length exceeds std::string max size.");

        int len = 0;
        if (!Read(&len))
            return false;

        if (len < 0 || len > c_maxStringLen)  // Other end is corrupted or is not following the protocol.
            throw ProgramError("Received bad string size.");

        if (len == 0)
        {
            *dest = "";
            return true;
        }

        dest->resize(static_cast<size_t>(len));
        return m_socket.Read(&*(dest->begin()), static_cast<size_t>(len));
    }
    catch (ProgramError const&)
    {
        if (!ec)
            throw;
        *ec = ErrorCode(std::current_exception());
        return false;
    }
}

}}  // namespace strapper::net
