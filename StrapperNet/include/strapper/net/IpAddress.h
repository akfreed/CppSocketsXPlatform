// ==================================================================
// Copyright 2021-2022 Alexander K. Freed
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

#include <array>
#include <cstdint>
#include <string>

namespace strapper { namespace net {

class IpAddressV4
{
public:
    static IpAddressV4 const Any;
    static IpAddressV4 const Loopback;

    IpAddressV4() = default;
    explicit IpAddressV4(std::string const& ip);
    //! @param[in] val An int (in network byte order) representation of an IP address.
    explicit IpAddressV4(uint32_t val);

    std::string ToString(char delim = ':') const;
    std::array<uint8_t, 4> ToArray() const;
    //! @return The address as an int in network byte order.
    uint32_t ToInt() const;

private:
    uint32_t m_val = 0;
};

}}  // namespace strapper::net
