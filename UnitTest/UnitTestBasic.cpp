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

#include <strapper/net/TcpListener.h>
#include <strapper/net/TcpSerializer.h>
#include <strapper/net/TcpSocket.h>
#include <strapper/net/UdpSocket.h>
#include <strapper/net/IpAddress.h>
#include "Timeout.h"
#include "TestGlobals.h"

#include <cstring>
#include <string>
#include <memory>
#include <chrono>
#include <algorithm>
#include <thread>
#include <vector>

namespace strapper { namespace net { namespace test {

class UnitTestBasic : public ::testing::Test
{
    Timeout m_timeout{std::chrono::seconds(3)};
};

TEST_F(UnitTestBasic, Empty)
{ }

TEST_F(UnitTestBasic, TcpSelfConnect)
{
    TcpListener listener(TestGlobals::port);
    ASSERT_TRUE(listener);
    TcpSocket client(TestGlobals::localhost, TestGlobals::port);
    ASSERT_TRUE(client.IsConnected());
    TcpSocket host = listener.Accept();
    ASSERT_TRUE(host.IsConnected());
}

TEST_F(UnitTestBasic, UdpCreate)
{
    UdpSocket client(0);
    ASSERT_TRUE(client);
    UdpSocket host(TestGlobals::port);
    ASSERT_TRUE(host);
}

TEST_F(UnitTestBasic, TcpSendRecvBuf)
{
    TcpListener listener(TestGlobals::port);
    ASSERT_TRUE(listener);
    TcpSocket sender(TestGlobals::localhost, TestGlobals::port);
    ASSERT_TRUE(sender.IsConnected());
    TcpSocket receiver = listener.Accept();
    ASSERT_TRUE(receiver.IsConnected());

    char sentData[6] = { 1, 2, 3, 4, 5, 6 };
    sender.Write(sentData, 5);

    char recvData[6] = { 0, 0, 0, 0, 0, 0 };
    ASSERT_TRUE(receiver.Read(recvData, 5));
    ASSERT_TRUE(std::equal(recvData, recvData + 5, sentData));

    sender.Write(sentData + 3, 3);
    sender.Write(sentData, 3);

    ASSERT_TRUE(receiver.Read(recvData, 2));
    std::vector<int> expected = { 4, 5, 3, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));

    ASSERT_TRUE(receiver.Read(recvData, 4));
    expected = { 6, 1, 2, 3, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
}

TEST_F(UnitTestBasic, UdpSendRecvBuf)
{
    IpAddressV4 const ip(TestGlobals::localhost);
    uint16_t const port = TestGlobals::port;
    UdpSocket sender(TestGlobals::port2);
    ASSERT_TRUE(sender);
    UdpSocket receiver(TestGlobals::port);
    ASSERT_TRUE(receiver);

    char sentData[6] = { 1, 2, 3, 4, 5, 6 };
    sender.Write(sentData, 5, ip, port);

    char recvData[6] = { 0, 0, 0, 0, 0, 0 };
    uint16_t senderPort = 0;
    ASSERT_EQ(receiver.Read(recvData, 5, nullptr, nullptr), 5);
    ASSERT_TRUE(std::equal(recvData, recvData + 5, sentData));

    sender.Write(sentData + 3, 3, ip, port);
    sender.Write(sentData, 3, ip, port);

    IpAddressV4 senderIp;
    ASSERT_EQ(receiver.Read(recvData, 3, &senderIp, &senderPort), 3);
    std::vector<char> expected = { 4, 5, 6, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
    ASSERT_EQ(senderIp.ToString(), ip.ToString());

    ASSERT_EQ(receiver.Read(recvData, 40, &senderIp, &senderPort), 3);
    expected = { 1, 2, 3, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
    ASSERT_EQ(senderIp.ToString(), ip.ToString());
}

// Test the DataAvailable() function.
TEST_F(UnitTestBasic, DataAvailable)
{
    TcpListener listener(TestGlobals::port);
    ASSERT_TRUE(listener);
    TcpSerializer sender(TcpSocket(TestGlobals::localhost, TestGlobals::port));
    ASSERT_TRUE(sender.Socket().IsConnected());
    TcpSerializer receiver(TcpSocket(listener.Accept()));
    ASSERT_TRUE(receiver.Socket().IsConnected());

    ASSERT_EQ(receiver.Socket().DataAvailable(), 0u);

    int i = 5;
    sender.Write(i);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_GT(receiver.Socket().DataAvailable(), 0u);
    ASSERT_TRUE(receiver.Read(&i));
    ASSERT_EQ(receiver.Socket().DataAvailable(), 0u);
}

} } }
