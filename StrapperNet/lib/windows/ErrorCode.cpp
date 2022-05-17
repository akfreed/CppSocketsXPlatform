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

#include "SocketIncludes.h"
#include <strapper/net/ErrorCode.h>

#include <strapper/net/SocketError.h>

#include <cassert>
#include <string>

namespace strapper { namespace net {

#define CODES(FUNC)              \
    FUNC(NO_ERROR)               \
    FUNC(WSA_NOT_ENOUGH_MEMORY)  \
    FUNC(WSAEACCES)              \
    FUNC(WSAEADDRINUSE)          \
    FUNC(WSAEADDRNOTAVAIL)       \
    FUNC(WSAEAFNOSUPPORT)        \
    FUNC(WSAEALREADY)            \
    FUNC(WSAECONNABORTED)        \
    FUNC(WSAECONNREFUSED)        \
    FUNC(WSAECONNRESET)          \
    FUNC(WSAEFAULT)              \
    FUNC(WSAEHOSTUNREACH)        \
    FUNC(WSAEINPROGRESS)         \
    FUNC(WSAEINTR)               \
    FUNC(WSAEINVAL)              \
    FUNC(WSAEINVALIDPROCTABLE)   \
    FUNC(WSAEINVALIDPROVIDER)    \
    FUNC(WSAEISCONN)             \
    FUNC(WSAEMFILE)              \
    FUNC(WSAEMSGSIZE)            \
    FUNC(WSAENETDOWN)            \
    FUNC(WSAENETRESET)           \
    FUNC(WSAEPROTONOSUPPORT)     \
    FUNC(WSAENETUNREACH)         \
    FUNC(WSAEPROVIDERFAILEDINIT) \
    FUNC(WSAENOBUFS)             \
    FUNC(WSAENOPROTOOPT)         \
    FUNC(WSAENOTCONN)            \
    FUNC(WSAENOTSOCK)            \
    FUNC(WSAEOPNOTSUPP)          \
    FUNC(WSAEPROCLIM)            \
    FUNC(WSAEPROTOTYPE)          \
    FUNC(WSAESHUTDOWN)           \
    FUNC(WSAESOCKTNOSUPPORT)     \
    FUNC(WSAETIMEDOUT)           \
    FUNC(WSAEWOULDBLOCK)         \
    FUNC(WSAHOST_NOT_FOUND)      \
    FUNC(WSANO_DATA)             \
    FUNC(WSANO_RECOVERY)         \
    FUNC(WSANOTINITIALISED)      \
    FUNC(WSASYSNOTREADY)         \
    FUNC(WSATRY_AGAIN)           \
    FUNC(WSATYPE_NOT_FOUND)      \
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

ErrorCode::ErrorCode(std::exception_ptr exception)  // NOLINT(performance-unnecessary-value-param): Clang-tidy thinks this is a const reference. I don't know what's going on under the hood, but I prefer value semantics with std::exception_ptr for safety purposes.
    : m_exception(std::move(exception))             // NOLINT(hicpp-move-const-arg, performance-move-const-arg)
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

}}  // namespace strapper::net
