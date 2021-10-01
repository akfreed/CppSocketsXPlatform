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

#include <strapper/net/ErrorCode.h>

#include <strapper/net/SocketError.h>

#include <sys/socket.h>
#include <netdb.h>

#include <cassert>
#include <string>

namespace strapper { namespace net {

#define CODES(FUNC) \
    FUNC(EACCES) \
    FUNC(EADDRINUSE) \
    FUNC(EADDRNOTAVAIL) \
    FUNC(EAFNOSUPPORT) \
    FUNC(EAGAIN) \
    FUNC(EAI_ADDRFAMILY) \
    FUNC(EAI_AGAIN) \
    FUNC(EAI_BADFLAGS) \
    FUNC(EAI_FAIL) \
    FUNC(EAI_FAMILY) \
    FUNC(EAI_MEMORY) \
    FUNC(EAI_NODATA) \
    FUNC(EAI_NONAME) \
    FUNC(EAI_SERVICE) \
    FUNC(EAI_SOCKTYPE) \
    FUNC(EAI_SYSTEM) \
    FUNC(EALREADY) \
    FUNC(EBADF) \
    FUNC(ECONNABORTED) \
    FUNC(ECONNREFUSED) \
    FUNC(ECONNRESET) \
    FUNC(EDESTADDRREQ) \
    FUNC(EDOM) \
    FUNC(EDQUOT) \
    FUNC(EFAULT) \
    FUNC(EHOSTUNREACH) \
    FUNC(EINPROGRESS) \
    FUNC(EINTR) \
    FUNC(EINVAL) \
    FUNC(EIO) \
    FUNC(EISCONN) \
    FUNC(ELOOP) \
    FUNC(EMFILE) \
    FUNC(EMSGSIZE) \
    FUNC(ENAMETOOLONG) \
    FUNC(ENETUNREACH) \
    FUNC(ENFILE) \
    FUNC(ENOBUFS) \
    FUNC(ENOENT) \
    FUNC(ENOMEM) \
    FUNC(ENOPROTOOPT) \
    FUNC(ENOSPC) \
    FUNC(ENOSR) \
    FUNC(ENOTCONN) \
    FUNC(ENOTDIR) \
    FUNC(ENOTSOCK) \
    FUNC(EOPNOTSUPP) \
    FUNC(EPERM) \
    FUNC(EPIPE) \
    FUNC(EPROTONOSUPPORT) \
    FUNC(EPROTOTYPE) \
    FUNC(EROFS) \
    FUNC(ESOCKTNOSUPPORT) \
    FUNC(ETIMEDOUT) \

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
