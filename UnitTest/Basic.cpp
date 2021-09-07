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

#include <TcpListener.h>
#include <TcpSerializer.h>
#include <TcpSocket.h>
#include <UdpSocket.h>
#include "Timeout.h"
#include "TestGlobals.h"

#include <cstring>
#include <string>
#include <memory>
#include <chrono>
#include <algorithm>
#include <thread>

class Basic : public ::testing::Test
{
    Timeout m_timeout{std::chrono::seconds(3)};
};

TEST_F(Basic, TcpSelfConnect)
{
    TcpListener listener(TestGlobals::portString);
    ASSERT_TRUE(listener.IsValid());
    TcpSocket client(TestGlobals::localhost, TestGlobals::portString);
    ASSERT_TRUE(client.IsConnected());
    TcpSocket host = listener.Accept();
    ASSERT_TRUE(host.IsConnected());
}

TEST_F(Basic, UdpSelfConnect)
{
    UdpSocket client(TestGlobals::localhost, TestGlobals::port);
    ASSERT_TRUE(client.IsValid());
    UdpSocket host(TestGlobals::port);
    ASSERT_TRUE(host.IsValid());
}

TEST_F(Basic, TcpSendRecvBuf)
{
    TcpListener listener(TestGlobals::portString);
    ASSERT_TRUE(listener.IsValid());
    TcpSocket sender(TestGlobals::localhost, TestGlobals::portString);
    ASSERT_TRUE(sender.IsConnected());
    TcpSocket receiver = listener.Accept();
    ASSERT_TRUE(receiver.IsConnected());

    char sentData[6] = { 1, 2, 3, 4, 5, 6 };
    ASSERT_TRUE(sender.Write(sentData, 5));

    char recvData[6] = { 0, 0, 0, 0, 0, 0 };
    ASSERT_TRUE(receiver.Read(recvData, 5));
    ASSERT_TRUE(std::equal(recvData, recvData + 5, sentData));

    ASSERT_TRUE(sender.Write(sentData + 3, 3));
    ASSERT_TRUE(sender.Write(sentData, 3));

    ASSERT_TRUE(receiver.Read(recvData, 2));
    auto expected = { 4, 5, 3, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));

    ASSERT_TRUE(receiver.Read(recvData, 4));
    expected = { 6, 1, 2, 3, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
}

TEST_F(Basic, UdpSendRecvBuf)
{
    UdpSocket sender(TestGlobals::localhost, TestGlobals::port);
    ASSERT_TRUE(sender.IsValid());
    UdpSocket receiver(TestGlobals::port);
    ASSERT_TRUE(receiver.IsValid());

    char sentData[6] = { 1, 2, 3, 4, 5, 6 };
    ASSERT_TRUE(sender.Write(sentData, 5));

    char recvData[6] = { 0, 0, 0, 0, 0, 0 };
    ASSERT_TRUE(receiver.Read(recvData, 5));
    ASSERT_TRUE(std::equal(recvData, recvData + 5, sentData));

    ASSERT_TRUE(sender.Write(sentData + 3, 3));
    ASSERT_TRUE(sender.Write(sentData, 3));

    ASSERT_TRUE(receiver.Read(recvData, 2));
    auto expected = { 4, 5, 3, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));

    ASSERT_TRUE(receiver.Read(recvData, 4));
    expected = { 1, 2, 3, 4, 5, 0 };
    ASSERT_TRUE(std::equal(recvData, recvData + 6, expected.begin()));
}

// Test the DataAvailable() function.
TEST_F(Basic, DataAvailable)
{
    TcpListener listener(TestGlobals::portString);
    ASSERT_TRUE(listener.IsValid());
    TcpSerializer sender(TcpSocket(TestGlobals::localhost, TestGlobals::portString));
    ASSERT_TRUE(sender.Socket().IsConnected());
    TcpSerializer receiver(TcpSocket(listener.Accept()));
    ASSERT_TRUE(receiver.Socket().IsConnected());

    ASSERT_EQ(receiver.Socket().DataAvailable(), 0);

    int i = 5;
    ASSERT_TRUE(sender.Write(i));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_GT(receiver.Socket().DataAvailable(), 0);
    ASSERT_TRUE(receiver.Read(i));
    ASSERT_EQ(receiver.Socket().DataAvailable(), 0);
}

