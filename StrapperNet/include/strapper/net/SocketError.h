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

#pragma once

#include <strapper/net/ErrorCode.h>

#include <exception>
#include <string>

namespace strapper { namespace net {

class ProgramError : public std::exception
{
public:
    ProgramError() = default;

    explicit ProgramError(std::string what)
        : m_what(std::move(what))
    { }

    char const* what() const noexcept override { return m_what.c_str(); }

protected:
    std::string m_what = "Program Error.";
};

class SocketError : public ProgramError
{
public:
    SocketError()
        : SocketError(0)
    { }

    explicit SocketError(int nativeErrorCode)
        : ProgramError(nativeErrorCode != 0 ? "SocketError: A socket API call returned " + ErrorCode::GetErrorName(nativeErrorCode) + "."
                                            : "SocketError: Unknown cause.")
        , m_nativeCode(nativeErrorCode)
    { }

    int NativeCode() const
    {
        return m_nativeCode;
    }

protected:
    int m_nativeCode = 0;
};

}}  // namespace strapper::net
