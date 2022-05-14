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

#include <strapper/net/IpAddress.h>

#include <strapper/net/SocketError.h>

#include <cstring>
#include <limits>
#include <regex>
#include <sstream>

namespace strapper { namespace net {

IpAddressV4 const IpAddressV4::Any{};
IpAddressV4 const IpAddressV4::Loopback{ "127.0.0.1" };

IpAddressV4::IpAddressV4(std::string const& ip)
{
    ProgramError error("Not a valid IPv4 address: '" + ip + "'");
    if (!std::regex_match(ip, std::regex(R"(^(\d{1,3}[:\.]){3}\d{1,3}$)")))
        throw error;

    std::regex const r(R"(\d{1,3}(:|.|$))");
    auto iter = std::sregex_iterator(ip.cbegin(), ip.cend(), r);
    auto end = std::sregex_iterator();

    std::array<uint8_t, 4> array{};

    for (auto& e : array)
    {
        if (iter == end)
            throw error;
        unsigned long ul = 0;
        try
        {
            ul = std::stoul((iter++)->str());
        }
        catch (std::logic_error const&)
        {
            throw error;
        }

        if (ul > std::numeric_limits<uint8_t>::max())
            throw error;

        e = static_cast<uint8_t>(ul);  // Big endian.
    }

    std::memcpy(&m_val, array.data(), 4);
}

IpAddressV4::IpAddressV4(uint32_t val)
    : m_val(val)
{ }

std::string IpAddressV4::ToString(char delim /* = ':' */) const
{
    std::stringstream s;
    auto const array = ToArray();
    s << std::to_string(array[0]) << delim
      << std::to_string(array[1]) << delim
      << std::to_string(array[2]) << delim
      << std::to_string(array[3]);
    return s.str();
}

std::array<uint8_t, 4> IpAddressV4::ToArray() const
{
    std::array<uint8_t, 4> ret{};
    std::memcpy(ret.data(), &m_val, 4);
    return ret;
}

uint32_t IpAddressV4::ToInt() const
{
    return m_val;
}

}}  // namespace strapper::net
