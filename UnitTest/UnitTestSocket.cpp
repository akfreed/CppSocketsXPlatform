// ==================================================================
// Copyright 2018-2022 Alexander K. Freed
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
#include <thread>
#include <vector>

namespace strapper { namespace net { namespace test {

class UnitTestSocket : public ::testing::Test
{ };

TEST_F(UnitTestSocket, Empty)
{ }

TEST_F(UnitTestSocket, SelfConnectTcp)
{
    Timeout timeout(std::chrono::seconds(3));

    TcpListener listener(TestGlobals::port);
    ASSERT_TRUE(listener);
    TcpSocket client(TestGlobals::localhost, TestGlobals::port);
    ASSERT_TRUE(client.IsConnected());
    TcpSocket host = listener.Accept();
    ASSERT_TRUE(host.IsConnected());
}

TEST_F(UnitTestSocket, SelfConnectTcpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    ErrorCode ec;
    TcpListener listener(TestGlobals::port, &ec);
    ASSERT_TRUE(listener);
    ASSERT_FALSE(ec);
    TcpSocket client(TestGlobals::localhost, TestGlobals::port, &ec);
    ASSERT_TRUE(client.IsConnected());
    ASSERT_FALSE(ec);
    TcpSocket host = listener.Accept(&ec);
    ASSERT_FALSE(ec);
    ASSERT_TRUE(host.IsConnected());
}

TEST_F(UnitTestSocket, CreateUdp)
{
    Timeout timeout(std::chrono::seconds(3));

    UdpSocket client(0);
    ASSERT_TRUE(client);
    UdpSocket host(TestGlobals::port);
    ASSERT_TRUE(host);
}

TEST_F(UnitTestSocket, CreateUdpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    ErrorCode ec;
    UdpSocket client(0, &ec);
    ASSERT_TRUE(client);
    ASSERT_FALSE(ec);
    UdpSocket host(TestGlobals::port, &ec);
    ASSERT_TRUE(host);
    ASSERT_FALSE(ec);
}

TEST_F(UnitTestSocket, SendRecvBufTcp)
{
    Timeout timeout(std::chrono::seconds(3));

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

TEST_F(UnitTestSocket, SendRecvBufTcpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    ErrorCode ec;
    TcpListener listener(TestGlobals::port, &ec);
    ASSERT_TRUE(listener);
    ASSERT_FALSE(ec);
    TcpSocket sender(TestGlobals::localhost, TestGlobals::port, &ec);
    ASSERT_TRUE(sender.IsConnected());
    ASSERT_FALSE(ec);
    TcpSocket receiver = listener.Accept(&ec);
    ASSERT_TRUE(receiver.IsConnected());
    ASSERT_FALSE(ec);

    char sentData[6] = { 1, 2, 3, 4, 5, 6 };
    sender.Write(sentData, 5, &ec);
    ASSERT_FALSE(ec);

    char recvData[6] = { 0, 0, 0, 0, 0, 0 };
    ASSERT_TRUE(receiver.Read(recvData, 5, &ec));
    ASSERT_FALSE(ec);
    ASSERT_TRUE(std::equal(recvData, recvData + 5, sentData));

    sender.Write(sentData + 3, 3, &ec);
    ASSERT_FALSE(ec);
    sender.Write(sentData, 3, &ec);
    ASSERT_FALSE(ec);

    ASSERT_TRUE(receiver.Read(recvData, 2, &ec));
    ASSERT_FALSE(ec);
    std::vector<int> expected = { 4, 5, 3, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));

    ASSERT_TRUE(receiver.Read(recvData, 4, &ec));
    ASSERT_FALSE(ec);
    expected = { 6, 1, 2, 3, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
}

TEST_F(UnitTestSocket, SendRecvUdpBuf)
{
    Timeout timeout(std::chrono::seconds(3));

    IpAddressV4 const ip(TestGlobals::localhost);
    uint16_t const port = TestGlobals::port;
    UdpSocket sender(TestGlobals::port2);
    ASSERT_TRUE(sender);
    UdpSocket receiver(TestGlobals::port);
    ASSERT_TRUE(receiver);

    char const sentData[6] = { 1, 2, 3, 4, 5, 6 };
    sender.Write(sentData, 5, ip, port);

    char recvData[6] = { 0, 0, 0, 0, 0, 0 };
    ASSERT_EQ(receiver.Read(recvData, 5, nullptr, nullptr), 5);
    ASSERT_TRUE(std::equal(recvData, recvData + 5, sentData));

    sender.Write(sentData + 3, 3, ip, port);
    sender.Write(sentData, 3, ip, port);

    IpAddressV4 senderIp;
    uint16_t senderPort = 0;
    ASSERT_EQ(receiver.Read(recvData, 3, &senderIp, &senderPort), 3);
    std::vector<char> expected = { 4, 5, 6, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
    ASSERT_EQ(senderIp.ToString(), ip.ToString());
    ASSERT_EQ(senderPort, TestGlobals::port2);

    ASSERT_EQ(receiver.Read(recvData, 40, &senderIp, &senderPort), 3);
    expected = { 1, 2, 3, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
    ASSERT_EQ(senderIp.ToString(), ip.ToString());
    ASSERT_EQ(senderPort, TestGlobals::port2);

    std::fill(recvData, recvData + 6, 0);
    receiver.Write(sentData, 2, ip, TestGlobals::port2);
    ASSERT_EQ(sender.Read(recvData, 40, &senderIp, &senderPort), 2);
    expected = { 1, 2, 0, 0, 0, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
    ASSERT_EQ(senderIp.ToString(), ip.ToString());
    ASSERT_EQ(senderPort, port);
}

TEST_F(UnitTestSocket, SendRecvBufUdpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    IpAddressV4 const ip(TestGlobals::localhost);
    uint16_t const port = TestGlobals::port;
    ErrorCode ec;
    UdpSocket sender(TestGlobals::port2, &ec);
    ASSERT_TRUE(sender);
    ASSERT_FALSE(ec);
    UdpSocket receiver(TestGlobals::port, &ec);
    ASSERT_TRUE(receiver);
    ASSERT_FALSE(ec);

    char const sentData[6] = { 1, 2, 3, 4, 5, 6 };
    sender.Write(sentData, 5, ip, port, &ec);
    ASSERT_FALSE(ec);

    char recvData[6] = { 0, 0, 0, 0, 0, 0 };
    ASSERT_EQ(receiver.Read(recvData, 5, nullptr, nullptr, &ec), 5);
    ASSERT_FALSE(ec);
    ASSERT_TRUE(std::equal(recvData, recvData + 5, sentData));

    sender.Write(sentData + 3, 3, ip, port, &ec);
    ASSERT_FALSE(ec);
    sender.Write(sentData, 3, ip, port, &ec);
    ASSERT_FALSE(ec);

    IpAddressV4 senderIp;
    uint16_t senderPort = 0;
    ASSERT_EQ(receiver.Read(recvData, 3, &senderIp, &senderPort, &ec), 3);
    ASSERT_FALSE(ec);
    std::vector<char> expected = { 4, 5, 6, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
    ASSERT_EQ(senderIp.ToString(), ip.ToString());
    ASSERT_EQ(senderPort, TestGlobals::port2);

    ASSERT_EQ(receiver.Read(recvData, 40, &senderIp, &senderPort, &ec), 3);
    ASSERT_FALSE(ec);
    expected = { 1, 2, 3, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
    ASSERT_EQ(senderIp.ToString(), ip.ToString());
    ASSERT_EQ(senderPort, TestGlobals::port2);

    std::fill(recvData, recvData + 6, 0);
    receiver.Write(sentData, 2, ip, TestGlobals::port2, &ec);
    ASSERT_FALSE(ec);
    ASSERT_EQ(sender.Read(recvData, 40, &senderIp, &senderPort, &ec), 2);
    ASSERT_FALSE(ec);
    expected = { 1, 2, 0, 0, 0, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
    ASSERT_EQ(senderIp.ToString(), ip.ToString());
    ASSERT_EQ(senderPort, port);
}

// Test the DataAvailable() function.
TEST_F(UnitTestSocket, DataAvailableTcp)
{
    Timeout timeout(std::chrono::seconds(3));

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

// Test the DataAvailable() function.
TEST_F(UnitTestSocket, DataAvailableTcpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    ErrorCode ec;
    TcpListener listener(TestGlobals::port, &ec);
    ASSERT_TRUE(listener);
    ASSERT_FALSE(ec);
    TcpSerializer sender(TcpSocket(TestGlobals::localhost, TestGlobals::port, &ec));
    ASSERT_TRUE(sender.Socket().IsConnected());
    ASSERT_FALSE(ec);
    TcpSerializer receiver(TcpSocket(listener.Accept(&ec)));
    ASSERT_TRUE(receiver.Socket().IsConnected());
    ASSERT_FALSE(ec);

    ASSERT_EQ(receiver.Socket().DataAvailable(&ec), 0u);
    ASSERT_FALSE(ec);

    int i = 5;
    sender.Write(i);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_GT(receiver.Socket().DataAvailable(&ec), 0u);
    ASSERT_FALSE(ec);
    ASSERT_TRUE(receiver.Read(&i));
    ASSERT_EQ(receiver.Socket().DataAvailable(&ec), 0u);
    ASSERT_FALSE(ec);
}

// Test the DataAvailable() function.
TEST_F(UnitTestSocket, DataAvailableUdp)
{
    Timeout timeout(std::chrono::seconds(3));

    UdpSocket sender(TestGlobals::port2);
    ASSERT_TRUE(sender);
    UdpSocket receiver(TestGlobals::port);
    ASSERT_TRUE(receiver);

    IpAddressV4 senderIp;
    uint16_t senderPort = 0;

    {
        ASSERT_EQ(receiver.DataAvailable(), 0u);

        int const toWrite = 5;
        sender.Write(&toWrite, sizeof(toWrite), IpAddressV4::Loopback, TestGlobals::port);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        int toRead = 0;
        static_assert(sizeof(toWrite) == sizeof(toRead), "Buffers must be the same size.");
        ASSERT_EQ(receiver.DataAvailable(), sizeof(toWrite));
        ASSERT_EQ(receiver.Read(&toRead, 100, &senderIp, &senderPort), sizeof(toWrite));
        ASSERT_EQ(toRead, toWrite);
        ASSERT_EQ(receiver.DataAvailable(), 0u);
        ASSERT_EQ(senderIp.ToString(), IpAddressV4::Loopback.ToString());
        ASSERT_EQ(senderPort, TestGlobals::port2);
    }

    {
        ASSERT_EQ(sender.DataAvailable(), 0u);

        double const toWrite = 5.1;
        receiver.Write(&toWrite, sizeof(toWrite), IpAddressV4::Loopback, TestGlobals::port2);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        double toRead = 0;
        static_assert(sizeof(toWrite) == sizeof(toRead), "Buffers must be the same size.");
        ASSERT_EQ(sender.DataAvailable(), sizeof(toWrite));
        ASSERT_EQ(sender.Read(&toRead, 1000, &senderIp, &senderPort), sizeof(toWrite));
        ASSERT_EQ(toRead, toWrite);
        ASSERT_EQ(sender.DataAvailable(), 0u);
        ASSERT_EQ(senderIp.ToInt(), IpAddressV4::Loopback.ToInt());
        ASSERT_EQ(senderPort, TestGlobals::port);
    }
}

// Test the DataAvailable() function.
TEST_F(UnitTestSocket, DataAvailableUdpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    ErrorCode ec;
    UdpSocket sender(TestGlobals::port2, &ec);
    ASSERT_TRUE(sender);
    ASSERT_FALSE(ec);
    UdpSocket receiver(TestGlobals::port, &ec);
    ASSERT_TRUE(receiver);
    ASSERT_FALSE(ec);

    IpAddressV4 senderIp;
    uint16_t senderPort = 0;

    {
        ASSERT_EQ(receiver.DataAvailable(&ec), 0u);
        ASSERT_FALSE(ec);

        int const toWrite = 5;
        sender.Write(&toWrite, sizeof(toWrite), IpAddressV4::Loopback, TestGlobals::port, &ec);
        ASSERT_FALSE(ec);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        int toRead = 0;
        static_assert(sizeof(toWrite) == sizeof(toRead), "Buffers must be the same size.");
        ASSERT_EQ(receiver.DataAvailable(&ec), sizeof(toWrite));
        ASSERT_FALSE(ec);
        ASSERT_EQ(receiver.Read(&toRead, 100, &senderIp, &senderPort, &ec), sizeof(toWrite));
        ASSERT_FALSE(ec);
        ASSERT_EQ(toRead, toWrite);
        ASSERT_EQ(receiver.DataAvailable(&ec), 0u);
        ASSERT_FALSE(ec);
        ASSERT_EQ(senderIp.ToString(), IpAddressV4::Loopback.ToString());
        ASSERT_EQ(senderPort, TestGlobals::port2);
    }

    {
        ASSERT_EQ(sender.DataAvailable(&ec), 0u);
        ASSERT_FALSE(ec);

        double const toWrite = 5.1;
        receiver.Write(&toWrite, sizeof(toWrite), IpAddressV4::Loopback, TestGlobals::port2, &ec);
        ASSERT_FALSE(ec);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        double toRead = 0;
        static_assert(sizeof(toWrite) == sizeof(toRead), "Buffers must be the same size.");
        ASSERT_EQ(sender.DataAvailable(&ec), sizeof(toWrite));
        ASSERT_FALSE(ec);
        ASSERT_EQ(sender.Read(&toRead, 1000, &senderIp, &senderPort, &ec), sizeof(toWrite));
        ASSERT_FALSE(ec);
        ASSERT_EQ(toRead, toWrite);
        ASSERT_EQ(sender.DataAvailable(&ec), 0u);
        ASSERT_FALSE(ec);
        ASSERT_EQ(senderIp.ToInt(), IpAddressV4::Loopback.ToInt());
        ASSERT_EQ(senderPort, TestGlobals::port);
    }
}

}}} // namespace strapper::net::test
