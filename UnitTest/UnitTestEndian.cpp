// ==================================================================
// Copyright 2018-2021 Alexander K. Freed
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

#include <strapper/net/TcpListener.h>
#include <strapper/net/TcpSerializer.h>
#include <strapper/net/TcpSocket.h>
#include "TestGlobals.h"
#include "Timeout.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace strapper { namespace net { namespace test {

class UnitTestEndian : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        Timeout timeout{ std::chrono::seconds(3) };
        TcpListener listener(TestGlobals::port);
        ASSERT_TRUE(listener) << "Unable to start listener.";
        s_sender.reset(new TcpSerializer(TcpSocket(TestGlobals::localhost, TestGlobals::port)));
        ASSERT_TRUE(s_sender->Socket().IsConnected()) << "Unable to connect client to listener.";
        s_receiver.reset(new TcpSerializer(TcpSocket(listener.Accept())));
        ASSERT_TRUE(s_receiver->Socket().IsConnected()) << "Error on accept.";
    }

    static void TearDownTestSuite()
    {
        s_sender.reset();
        s_receiver.reset();
    }

    void SetUp() override
    {
        ASSERT_TRUE(s_sender);
        ASSERT_TRUE(s_sender->Socket().IsConnected());
        ASSERT_TRUE(s_receiver->Socket().IsConnected());
        // Since the sockets are re-used, a previous test failure can leave some data in the buffer.
        ASSERT_EQ(s_receiver->Socket().DataAvailable(), 0u) << "Receiver had data in buffer before data was sent.";
    }

    static std::unique_ptr<TcpSerializer> s_sender;
    static std::unique_ptr<TcpSerializer> s_receiver;

    Timeout m_timeout{ std::chrono::seconds(3) };
};

std::unique_ptr<TcpSerializer> UnitTestEndian::s_sender;
std::unique_ptr<TcpSerializer> UnitTestEndian::s_receiver;

TEST_F(UnitTestEndian, CheckInt)
{
    const int32_t value = 0x3CABBA6E;
    char valBuffer[sizeof(value)];

    std::memcpy(valBuffer, &value, sizeof(value));

    s_sender->Write(value);
    s_sender->Write(value);

    char readBuffer[sizeof(value)];
    ASSERT_TRUE(s_receiver->Socket().Read(readBuffer, sizeof(value)));
    char expected[4]{ '\x3C', '\xAB', '\xBA', '\x6E' };
    ASSERT_TRUE(std::equal(expected, expected + 4, readBuffer));

    int32_t readInt = 0;
    ASSERT_TRUE(s_receiver->Read(&readInt));
    std::memcpy(readBuffer, &readInt, sizeof(readInt));
    ASSERT_TRUE(std::equal(readBuffer, readBuffer + 4, valBuffer));
}

TEST_F(UnitTestEndian, CheckDouble)
{
    const uint64_t value = 0x0807060504030201;
    char valBuffer[sizeof(value)];
    double toSend;

    std::memcpy(valBuffer, &value, sizeof(value));
    std::memcpy(&toSend, &value, sizeof(value));

    s_sender->Write(toSend);
    s_sender->Write(toSend);

    char readBuffer[sizeof(value)];
    ASSERT_TRUE(s_receiver->Socket().Read(readBuffer, sizeof(value)));
    char expected[8]{ 8, 7, 6, 5, 4, 3, 2, 1 };
    ASSERT_TRUE(std::equal(expected, expected + 8, readBuffer));

    double readDouble = 0;
    ASSERT_TRUE(s_receiver->Read(&readDouble));
    std::memcpy(readBuffer, &readDouble, sizeof(readDouble));
    ASSERT_TRUE(std::equal(readBuffer, readBuffer + 8, valBuffer));
}

}}} // namespace strapper::net::test
