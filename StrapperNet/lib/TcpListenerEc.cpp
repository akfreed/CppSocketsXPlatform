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

#include <strapper/net/TcpListenerEc.h>

#include <strapper/net/SocketError.h>

namespace strapper { namespace net {

TcpListenerEc::TcpListenerEc(uint16_t port, ErrorCode* ec)
{
    try
    {
        m_listener = TcpListener(port);
    }
    catch (ProgramError const&)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
    }
}

TcpSocketEc::TcpSocketEc(TcpSocket&& socket)
    : m_socket(std::move(socket))
{ }

bool TcpListenerEc::IsListening() const
{
    return m_listener.IsListening();
}

void TcpListenerEc::Close()
{
    m_listener.Close();
}

TcpSocketEc TcpListenerEc::Accept(ErrorCode* ec)
{
    try
    {
        return TcpSocketEc(m_listener.Accept());
    }
    catch (ProgramError const&)
    {
        if (ec)
            *ec = ErrorCode(std::current_exception());
        return TcpSocketEc();
    }
}

TcpListenerEc::operator bool() const
{
    return !!m_listener;
}

TcpListener&& TcpListenerEc::Convert()
{
    return std::move(m_listener);
}

} }
