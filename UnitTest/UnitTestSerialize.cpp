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
#include <strapper/net/UdpSocket.h>
#include "TestGlobals.h"
#include "Timeout.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>
#include <string>

namespace strapper { namespace net { namespace test {

class UnitTestSerialize : public ::testing::Test
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

std::unique_ptr<TcpSerializer> UnitTestSerialize::s_sender;
std::unique_ptr<TcpSerializer> UnitTestSerialize::s_receiver;

TEST_F(UnitTestSerialize, SendRecvChar)
{
    const char sentData = 'f';
    s_sender->Write(sentData);
    char recvData{};
    ASSERT_TRUE(s_receiver->Read(&recvData));
    ASSERT_EQ(recvData, sentData);
}

TEST_F(UnitTestSerialize, SendRecvBool)
{
    s_sender->Write(false);
    s_sender->Write(true);
    constexpr bool s = true;
    s_sender->Write(s);

    bool b = true;
    ASSERT_TRUE(s_receiver->Read(&b));
    ASSERT_EQ(b, false);
    ASSERT_TRUE(s_receiver->Read(&b));
    ASSERT_EQ(b, true);
    ASSERT_TRUE(s_receiver->Read(&b));
    ASSERT_EQ(b, true);
}

TEST_F(UnitTestSerialize, SendRecvInt32)
{
    int sentData = -20;
    s_sender->Write(sentData);

    int recvData{};
    ASSERT_TRUE(s_receiver->Read(&recvData));
    ASSERT_EQ(recvData, sentData);
}

TEST_F(UnitTestSerialize, SendRecvDouble)
{
    double sentData = 5.1234567890;
    s_sender->Write(sentData);

    double recvData{};
    ASSERT_TRUE(s_receiver->Read(&recvData));
    ASSERT_EQ(recvData, sentData);
}

TEST_F(UnitTestSerialize, SendRecvCharString)
{
    std::string const out = "Hello, World!";
    s_sender->Write(out);

    std::string in;
    ASSERT_TRUE(s_receiver->Read(&in));
    ASSERT_EQ(in, out);
}

}}}  // namespace strapper::net::test
