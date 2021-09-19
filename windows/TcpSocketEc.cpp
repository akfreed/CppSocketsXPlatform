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

#include <TcpSocketEc.h>

#include <SocketError.h>

namespace strapper { namespace net {

TcpSocketEc::TcpSocketEc(std::string const& host, uint16_t port, ErrorCode* ec)
{
    try
    {
        m_socket = TcpSocket(host, port);
    }
    catch (ProgramError const&)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
    }
}

bool TcpSocketEc::IsConnected() const
{
    return m_socket.IsConnected();
}

void TcpSocketEc::SetReadTimeout(unsigned milliseconds, ErrorCode* ec)
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

void TcpSocketEc::ShutdownSend(ErrorCode* ec)
{
    try
    {
        m_socket.ShutdownSend();
    }
    catch (ProgramError const&)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
    }
}

void TcpSocketEc::ShutdownBoth()
{
    m_socket.ShutdownBoth();
}

void TcpSocketEc::Close()
{
    m_socket.Close();
}

void TcpSocketEc::Write(void const* src, size_t len, ErrorCode* ec)
{
    try
    {
        m_socket.Write(src, len);
    }
    catch (ProgramError const)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
    }
}

bool TcpSocketEc::Read(void* dest, size_t len, ErrorCode* ec)
{
    try
    {
        return m_socket.Read(dest, len);
    }
    catch (ProgramError const)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
        return false;
    }
}

unsigned TcpSocketEc::DataAvailable(ErrorCode* ec)
{
    try
    {
        return m_socket.DataAvailable();
    }
    catch (ProgramError const)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
        return 0;
    }
}

TcpSocketEc::operator bool() const
{
    return !!m_socket;
}

TcpSocket&& TcpSocketEc::Convert()
{
    return std::move(m_socket);
}

} }
