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

#include <strapper/net/TcpSocket.h>
#include <strapper/net/TcpSerializer.h>
#include <strapper/net/TcpListener.h>
#include <strapper/net/UdpSocket.h>
#include <strapper/net/SocketError.h>
#include <strapper/net/UdpSocket.h>
#include "TestGlobals.h"
#include "Timeout.h"

#include <thread>
#include <chrono>
#include <future>
#include <atomic>
#include <exception>

namespace strapper { namespace net { namespace test {

class UnitTestError : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
    }

    static void TearDownTestSuite()
    {
    }

    void SetUp() override
    {
        Timeout timeout{ std::chrono::seconds(3) };
        TcpListener listener(TestGlobals::port);
        ASSERT_TRUE(listener);
        m_sender = TcpSocket(TestGlobals::localhost, TestGlobals::port);
        ASSERT_TRUE(m_sender);
        m_receiver = listener.Accept();
        ASSERT_TRUE(m_receiver.IsConnected());
        ASSERT_EQ(m_receiver.DataAvailable(), 0u);
    }

    TcpSocket m_sender;
    TcpSocket m_receiver;
};

// Tests that SetReadTimeout breaks a blocking read and closes the socket.
TEST_F(UnitTestError, TcpReadTimeout)
{
    Timeout timeout(std::chrono::seconds(3));

    m_receiver.SetReadTimeout(500);

    auto const start = std::chrono::steady_clock::now();
    uint8_t buf[1];
    ASSERT_THROW(m_receiver.Read(buf, 1), SocketError) << "Socket read did not throw.";
    auto const stop = std::chrono::steady_clock::now();
    ASSERT_GT((stop - start), std::chrono::milliseconds(450)) << "Socket returned from read too early.";
    ASSERT_LT((stop - start), std::chrono::milliseconds(600)) << "Socket returend from read too late.";

    ASSERT_FALSE(m_receiver); // timing out should close the socket
}

TEST_F(UnitTestError, UdpReadTimeout)
{
    Timeout timeout(std::chrono::seconds(3));

    UdpSocket receiver(TestGlobals::port);
    receiver.SetReadTimeout(500);

    auto const start = std::chrono::steady_clock::now();
    uint8_t buf[1];
    ASSERT_THROW(receiver.Read(buf, 1, nullptr, nullptr), SocketError) << "Socket read did not throw.";
    auto const stop = std::chrono::steady_clock::now();
    ASSERT_GT((stop - start), std::chrono::milliseconds(450)) << "Socket returned from read too early.";
    ASSERT_LT((stop - start), std::chrono::milliseconds(600)) << "Socket returend from read too late.";

    ASSERT_TRUE(receiver); // timing out should not close the socket
}

// Tests that closing the socket breaks a blocking read on a separate thread.
TEST_F(UnitTestError, UnblockReadTcp)
{
    Timeout timeout(std::chrono::seconds(3));

    // Read on a separate thread.
    auto task = std::async(std::launch::async, [this]() {
        char buf[1];
        try
        {
            m_receiver.Read(buf, 1);
        }
        catch (ProgramError const& e)
        {
            std::cout << "Read failed successfully:\n" << e.what() << std::endl;
        }
    });

    // Make sure the read is still blocking.
    ASSERT_EQ(task.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout) << (m_receiver.IsConnected() ? "Socket returned from read too early." : "Socket closed.");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    m_receiver.Close(); // This call should block until the operation is done.
    ASSERT_FALSE(m_receiver);
    task.wait();
}

// Tests that closing the socket breaks a blocking read on a separate thread.
TEST_F(UnitTestError, UnblockReadUdp)
{
    Timeout timeout(std::chrono::seconds(3));

    // Read on a separate thread.
    UdpSocket receiver(TestGlobals::port);
    ASSERT_TRUE(receiver);
    auto task = std::async(std::launch::async, [&receiver]() {
        char buf[1];
        try
        {
            receiver.Read(buf, 1, nullptr, nullptr);
        }
        catch (ProgramError const& e)
        {
            std::cout << "Read failed successfully:\n" << e.what() << std::endl;
        }
    });

    // Make sure the read is still blocking.
    ASSERT_EQ(task.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout) << (receiver ? "Socket returned from read too early." : "Socket closed.");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    receiver.Close(); // This call should block until the operation is done.
    ASSERT_FALSE(receiver);
    task.wait();
}

// Tests that closing the socket breaks a blocking accept on a separate thread.
TEST_F(UnitTestError, UnblockAccept)
{
    //Timeout timeout(std::chrono::seconds(3));

    // Accept on a separate thread.
    TcpListener listener(TestGlobals::port);
    ASSERT_TRUE(listener);
    auto task = std::async(std::launch::async, [&listener]() {
        try
        {
            auto client = listener.Accept();
        }
        catch (ProgramError const& e)
        {
            std::cout << "Accept failed successfully:\n" << e.what() << std::endl;
        }
        });

    // Make sure the read is still blocking.
    ASSERT_EQ(task.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout) << (listener ? "Socket returned from read too early." : "Socket closed.");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    listener.Close(); // This call should block until the operation is done.
    ASSERT_FALSE(listener);
    task.wait();
}

} } }
