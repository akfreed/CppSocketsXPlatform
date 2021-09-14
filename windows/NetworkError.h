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

#include <ErrorCode.h>

#include <exception>
#include <string>

namespace strapper { namespace net {

class NetworkError : public std::exception
{
public:
    NetworkError() = default;
    NetworkError(NetworkError const&) = default;
    NetworkError& operator=(NetworkError const&) = default;
    explicit NetworkError(std::string message)
        : m_what(std::move(message))
    { }

    NetworkError(ErrorCode const& errorCode, std::string message)
        : m_what(std::move(message))
        , m_errorCode(errorCode)
    { }

    char const* what() const noexcept override
    {
        return m_what.c_str();
    }

private:
    std::string m_what = "Network Error thrown.";
    ErrorCode m_errorCode;
};

class NetworkProgrammingError : public NetworkError
{
public:
    using NetworkError::NetworkError;
};

class NetworkSystemError : public NetworkError
{
public:
    using NetworkError::NetworkError;
};

class NetworkConnectionError : public NetworkError
{
public:
    using NetworkError::NetworkError;
};

} }
