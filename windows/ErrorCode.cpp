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

#include <NetworkError.h>

#include <string>

#define CODES(L) \
    L(NO_ERROR, NetworkProgrammingError) \
    L(WSAEACCES, NetworkProgrammingError) \
    L(WSAECONNABORTED, NetworkConnectionError) \
    L(WSAECONNRESET, NetworkConnectionError) \
    L(WSAEFAULT, NetworkProgrammingError) \
    L(WSAEHOSTUNREACH, NetworkConnectionError) \
    L(WSAEINPROGRESS, NetworkProgrammingError) \
    L(WSAEINTR, NetworkProgrammingError) \
    L(WSAEINVAL, NetworkProgrammingError) \
    L(WSAEMFILE, NetworkSystemError) \
    L(WSAEMSGSIZE, NetworkProgrammingError) \
    L(WSAENETDOWN, NetworkSystemError) \
    L(WSAENETRESET, NetworkConnectionError) \
    L(WSAENOBUFS, NetworkSystemError) \
    L(WSAENOPROTOOPT, NetworkProgrammingError) \
    L(WSAENOTCONN, NetworkConnectionError) \
    L(WSAENOTSOCK, NetworkProgrammingError) \
    L(WSAEOPNOTSUPP, NetworkProgrammingError) \
    L(WSAESHUTDOWN, NetworkConnectionError) \
    L(WSAETIMEDOUT, NetworkConnectionError) \
    L(WSAEWOULDBLOCK, NetworkProgrammingError) \
    L(WSANOTINITIALISED, NetworkProgrammingError)

#define NAME_SWITCH(name, level) \
    case name: return #name;

#define LEVEL_SWITCH(name, level) \
    case name: return #level;

#define EXCEPT_SWITCH(name, level) \
    case name: throw level(*this, What());

namespace {

std::string GetName(int errorCode)
{
    switch (errorCode)
    {
        CODES(NAME_SWITCH)
    default:
        return std::to_string(errorCode);
    }
}

std::string GetLevel(int errorCode)
{
    switch (errorCode)
    {
        CODES(LEVEL_SWITCH)
    default:
        return "NetworkProgrammingError";
    }
}

}

ErrorCode::ErrorCode()
    : m_name(GetName(m_errorCode) + " (" + std::to_string(m_errorCode) + ")")
    , m_what(GetLevel(m_errorCode) + ": A socket API call returned " + m_name + ".")
{ }

ErrorCode::ErrorCode(int errorCode)
    : m_errorCode(errorCode)
    , m_name(GetName(m_errorCode) + " (" + std::to_string(m_errorCode) + ")")
    , m_what(GetLevel(m_errorCode) + ": A socket API call returned " + m_name + ".")
{ }

void ErrorCode::ThrowIfError() const
{
    if (!*this)
        return;

    switch (m_errorCode)
    {
        CODES(EXCEPT_SWITCH);
    default:
        throw NetworkProgrammingError(*this, What());
    }
}
