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

#include <strapper/net/UdpSocketEc.h>

#include <strapper/net/SocketError.h>

namespace strapper { namespace net {

UdpSocketEc::UdpSocketEc(uint16_t myport, ErrorCode* ec)
{
    try
    {
        m_socket = UdpSocket(myport);
    }
    catch (SocketError const&)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
    }
}

bool UdpSocketEc::IsOpen() const
{
    return m_socket.IsOpen();
}

void UdpSocketEc::SetReadTimeout(unsigned milliseconds, ErrorCode* ec)
{
    try
    {
        m_socket.SetReadTimeout(milliseconds);
    }
    catch (ProgramError const&)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
    }
}

void UdpSocketEc::Close()
{
    m_socket.Close();
}

void UdpSocketEc::Write(void const* src, size_t len, IpAddressV4 const& ipAddress, uint16_t port, ErrorCode* ec)
{
    try
    {
        m_socket.Write(src, len, ipAddress, port);
    }
    catch (ProgramError const&)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
    }
}

unsigned UdpSocketEc::Read(void* dest, size_t maxlen, IpAddressV4* out_ipAddress, uint16_t* out_port, ErrorCode* ec)
{
    try
    {
        return m_socket.Read(dest, maxlen, out_ipAddress, out_port);
    }
    catch (ProgramError const&)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
        return 0;
    }
}

unsigned UdpSocketEc::DataAvailable(ErrorCode* ec) const
{
    try
    {
        return m_socket.DataAvailable();
    }
    catch (ProgramError const&)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
        return 0;
    }
}

UdpSocketEc::operator bool() const
{
    return !!m_socket;
}

UdpSocket&& UdpSocketEc::Convert()
{
    return std::move(m_socket);
}

} }
