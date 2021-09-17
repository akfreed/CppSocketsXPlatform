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

#include <SocketIncludes.h>
#include <ErrorCode.h>

#include <SocketError.h>

#include <cassert>
#include <string>

namespace strapper { namespace net {

#define CODES(FUNC) \
    FUNC(NO_ERROR) \
    FUNC(WSAEACCES) \
    FUNC(WSAECONNABORTED) \
    FUNC(WSAECONNRESET) \
    FUNC(WSAEFAULT) \
    FUNC(WSAEHOSTUNREACH) \
    FUNC(WSAEINPROGRESS) \
    FUNC(WSAEINTR) \
    FUNC(WSAEINVAL) \
    FUNC(WSAEMFILE) \
    FUNC(WSAEMSGSIZE) \
    FUNC(WSAENETDOWN) \
    FUNC(WSAENETRESET) \
    FUNC(WSAENOBUFS) \
    FUNC(WSAENOPROTOOPT) \
    FUNC(WSAENOTCONN) \
    FUNC(WSAENOTSOCK) \
    FUNC(WSAEOPNOTSUPP) \
    FUNC(WSAEPROCLIM) \
    FUNC(WSAESHUTDOWN) \
    FUNC(WSAETIMEDOUT) \
    FUNC(WSAEWOULDBLOCK) \
    FUNC(WSANOTINITIALISED) \
    FUNC(WSASYSNOTREADY) \
    FUNC(WSAVERNOTSUPPORTED)

#define NAME_SWITCH(name) \
    case name: return #name " (" + std::to_string(name) + ")";

std::string ErrorCode::GetErrorName(int nativeErrorCode)
{
    switch (nativeErrorCode)
    {
        CODES(NAME_SWITCH)
    default:
        return std::to_string(nativeErrorCode);
    }
}

ErrorCode::ErrorCode(std::exception_ptr exception)
    : m_exception(std::move(exception))
{
    if (m_exception)
    {
        try
        {
            std::rethrow_exception(m_exception);
        }
        catch (SocketError const& e)
        {
            m_nativeErrorCode = e.NativeCode();
            m_what = e.what();
        }
        catch (ProgramError const& e)
        {
            m_what = e.what();
        }
    }
}

void ErrorCode::Rethrow() const
{
    assert(m_exception);
    std::rethrow_exception(m_exception);
}

} }
