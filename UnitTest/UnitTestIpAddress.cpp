// ==================================================================
// Copyright 2018, 2021 Alexander K. Freed
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

#include <gtest/gtest.h>

#include <strapper/net/IpAddress.h>
#include <strapper/net/SocketError.h>
#include <strapper/net/Endian.h>

#include <string>

namespace strapper { namespace net { namespace test {

class UnitTestIpAddress : public ::testing::Test
{
};

TEST_F(UnitTestIpAddress, IpAddressFail)
{
    ASSERT_THROW(IpAddressV4(""), ProgramError);
    ASSERT_THROW(IpAddressV4(":"), ProgramError);
    ASSERT_THROW(IpAddressV4("::"), ProgramError);
    ASSERT_THROW(IpAddressV4(":::"), ProgramError);
    ASSERT_THROW(IpAddressV4("::::"), ProgramError);
    ASSERT_THROW(IpAddressV4(":::::"), ProgramError);

    ASSERT_THROW(IpAddressV4("0:0:0"), ProgramError);
    ASSERT_THROW(IpAddressV4(":0:0:0"), ProgramError);
    ASSERT_THROW(IpAddressV4("0::0:0"), ProgramError);
    ASSERT_THROW(IpAddressV4("0:0::0"), ProgramError);
    ASSERT_THROW(IpAddressV4("0:0:0:"), ProgramError);

    ASSERT_THROW(IpAddressV4("256:0:0:0"), ProgramError);
    ASSERT_THROW(IpAddressV4("0:256:0:0"), ProgramError);
    ASSERT_THROW(IpAddressV4("0:0:256:0"), ProgramError);
    ASSERT_THROW(IpAddressV4("0:0:0:256"), ProgramError);

    ASSERT_THROW(IpAddressV4("1000:0:0:0"), ProgramError);
    ASSERT_THROW(IpAddressV4("0:999:0:0"), ProgramError);
    ASSERT_THROW(IpAddressV4("0:0:333:0"), ProgramError);
    ASSERT_THROW(IpAddressV4("0:0:0:1111111"), ProgramError);

    ASSERT_THROW(IpAddressV4("0:0:0:11111111111111111111111111111111"), ProgramError);

    ASSERT_THROW(IpAddressV4("0:0:0:A"), ProgramError);
    ASSERT_THROW(IpAddressV4("0:0:!:0"), ProgramError);
}

TEST_F(UnitTestIpAddress, ToString)
{
    ASSERT_EQ(IpAddressV4("0:0:0:0").ToString(), "0:0:0:0");
    ASSERT_EQ(IpAddressV4("255:255:255:255").ToString(), "255:255:255:255");
    ASSERT_EQ(IpAddressV4("1:2:3:4").ToString(), "1:2:3:4");
    ASSERT_NE(IpAddressV4("0:0:1:0").ToString(), "0:0:0:0");

    ASSERT_EQ(IpAddressV4("0.0:0:001").ToString('.'), "0.0.0.1");
}

TEST_F(UnitTestIpAddress, ToArray)
{
    using Array = std::array<uint8_t, 4>;
    ASSERT_TRUE((IpAddressV4("0:0:0:0").ToArray() == Array{ 0, 0, 0, 0 }));
    ASSERT_TRUE((IpAddressV4("255:255:255:255").ToArray() == Array{ 255, 255, 255, 255 }));
    ASSERT_TRUE((IpAddressV4("1:2:3:4").ToArray() == Array{ 1, 2, 3, 4 }));
}

TEST_F(UnitTestIpAddress, ToInt)
{
    ASSERT_EQ(IpAddressV4("0:0:0:0").ToInt(), 0u);
    ASSERT_EQ(IpAddressV4("255:255:255:255").ToInt(), 0xFFFFFFFFu);
    ASSERT_EQ(IpAddressV4("1:2:3:4").ToInt(), nton(0x01020304));
}

TEST_F(UnitTestIpAddress, Any)
{
    IpAddressV4 any = IpAddressV4::Any;
    ASSERT_EQ(any.ToString(), "0:0:0:0");
    ASSERT_TRUE((any.ToArray() == std::array<uint8_t, 4>{ 0, 0, 0, 0}));
    ASSERT_EQ(any.ToInt(), 0u);
}

TEST_F(UnitTestIpAddress, ConstructFromInt)
{
    ASSERT_EQ(IpAddressV4(0).ToInt(), 0u);
    ASSERT_EQ(IpAddressV4(0xFFFFFFFF).ToInt(), 0xFFFFFFFFu);
    ASSERT_EQ(IpAddressV4(nton(0xABCDEF01)).ToInt(), nton(0xABCDEF01u));
    ASSERT_EQ(IpAddressV4(nton(0xABCDEF01)).ToString(), "171:205:239:1");
}

} } }
