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

#include <string>

namespace strapper { namespace net {

class ErrorCode
{
public:
    explicit ErrorCode(int errorCode = 0);

    int NativeCode() const { return m_nativeErrorCode; }
    std::string const& Name() const { return m_name; }
    explicit operator bool() const { return m_nativeErrorCode != 0; }

    void ThrowIfError() const;
    void Throw() const;

private:
    int m_nativeErrorCode = 0;
    std::string m_name;
};

} }
