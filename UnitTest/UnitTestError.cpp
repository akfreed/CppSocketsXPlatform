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

#include <strapper/net/SocketError.h>
#include <strapper/net/TcpListener.h>
#include <strapper/net/TcpSocket.h>
#include <strapper/net/UdpSocket.h>
#include "TestGlobals.h"
#include "Timeout.h"

#include <atomic>
#include <chrono>
#include <exception>
#include <future>
#include <thread>

namespace strapper { namespace net { namespace test {

using std::chrono::milliseconds;

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
TEST_F(UnitTestError, ReadTimeoutTcp)
{
    Timeout timeout(std::chrono::seconds(3));

    m_receiver.SetReadTimeout(500);

    auto const start = std::chrono::steady_clock::now();
    uint8_t buf[1];
    ASSERT_THROW(m_receiver.Read(buf, 1), SocketError) << "Socket read did not throw.";
    auto const stop = std::chrono::steady_clock::now();
    ASSERT_GT((stop - start), milliseconds(450)) << "Socket returned from read too early.";
    ASSERT_LT((stop - start), milliseconds(600)) << "Socket returend from read too late.";

    ASSERT_FALSE(m_receiver);  // timing out should close the socket
}

// Tests that SetReadTimeout breaks a blocking read and closes the socket.
TEST_F(UnitTestError, ReadTimeoutTcpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    ErrorCode ec;
    m_receiver.SetReadTimeout(500, &ec);
    ASSERT_FALSE(ec);

    auto const start = std::chrono::steady_clock::now();
    uint8_t buf[1];
    ASSERT_NO_THROW(m_receiver.Read(buf, 1, &ec));
    auto const stop = std::chrono::steady_clock::now();
    ASSERT_TRUE(ec);
    ASSERT_THROW(ec.Rethrow(), SocketError) << "Socket read did not give error on timeout.";

    ASSERT_GT((stop - start), milliseconds(450)) << "Socket returned from read too early.";
    ASSERT_LT((stop - start), milliseconds(600)) << "Socket returend from read too late.";

    ASSERT_FALSE(m_receiver);  // timing out should close the socket
}

// Tests that SetReadTimeout breaks a blocking read and closes the socket.
TEST_F(UnitTestError, ReadTimeoutUdp)
{
    Timeout timeout(std::chrono::seconds(3));

    UdpSocket receiver(TestGlobals::port);
    receiver.SetReadTimeout(500);

    auto const start = std::chrono::steady_clock::now();
    uint8_t buf[1];
    ASSERT_THROW(receiver.Read(buf, 1, nullptr, nullptr), SocketError) << "Socket read did not throw.";
    auto const stop = std::chrono::steady_clock::now();
    ASSERT_GT((stop - start), milliseconds(450)) << "Socket returned from read too early.";
    ASSERT_LT((stop - start), milliseconds(600)) << "Socket returend from read too late.";

    ASSERT_TRUE(receiver);  // timing out should not close the socket
}

// Tests that SetReadTimeout breaks a blocking read and closes the socket.
TEST_F(UnitTestError, ReadTimeoutUdpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    ErrorCode ec;
    UdpSocket receiver(TestGlobals::port, &ec);
    ASSERT_FALSE(ec);
    receiver.SetReadTimeout(500, &ec);
    ASSERT_FALSE(ec);

    auto const start = std::chrono::steady_clock::now();
    uint8_t buf[1];
    ASSERT_NO_THROW(receiver.Read(buf, 1, nullptr, nullptr, &ec));
    auto const stop = std::chrono::steady_clock::now();
    ASSERT_TRUE(ec);
    ASSERT_THROW(ec.Rethrow(), SocketError) << "Socket read did not throw.";

    ASSERT_GT((stop - start), milliseconds(450)) << "Socket returned from read too early.";
    ASSERT_LT((stop - start), milliseconds(600)) << "Socket returend from read too late.";

    ASSERT_TRUE(receiver);  // timing out should not close the socket
}

// Tests that closing the socket breaks a blocking read on a separate thread.
TEST_F(UnitTestError, UnblockReadTcp)
{
    Timeout timeout(std::chrono::seconds(3));

    // Read on a separate thread.
    auto task = std::async(std::launch::async, [this]() -> bool {
        try
        {
            char buf[1];
            m_receiver.Read(buf, 1);
            std::cout << "Read did not fail as expected.\n"
                      << std::endl;
            return false;
        }
        catch (ProgramError const& e)
        {
            std::cout << "Read failed successfully:\n"
                      << e.what() << std::endl;
            return true;
        }
    });

    // Make sure the read is still blocking.
    ASSERT_EQ(task.wait_for(milliseconds(200)), std::future_status::timeout) << (m_receiver.IsConnected() ? "Socket returned from read too early." : "Socket closed.");
    std::this_thread::sleep_for(milliseconds(200));
    m_receiver.Close();  // This call should block until the operation is done.
    ASSERT_FALSE(m_receiver);
    ASSERT_TRUE(task.get());
}

// Tests that closing the socket breaks a blocking read on a separate thread.
TEST_F(UnitTestError, UnblockReadTcpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    // Read on a separate thread.
    auto task = std::async(std::launch::async, [this]() -> bool {
        char buf[1];
        ErrorCode ec;
        m_receiver.Read(buf, 1, &ec);

        try
        {
            if (ec)
                ec.Rethrow();
            std::cout << "Read did not fail as expected.\n"
                      << std::endl;
            return false;
        }
        catch (ProgramError const& e)
        {
            std::cout << "Read failed successfully:\n"
                      << e.what() << std::endl;
            return true;
        }
    });

    // Make sure the read is still blocking.
    ASSERT_EQ(task.wait_for(milliseconds(200)), std::future_status::timeout) << (m_receiver.IsConnected() ? "Socket returned from read too early." : "Socket closed.");
    std::this_thread::sleep_for(milliseconds(200));
    m_receiver.Close();  // This call should block until the operation is done.
    ASSERT_FALSE(m_receiver);
    ASSERT_TRUE(task.get());
}

// Tests that closing the socket breaks a blocking read on a separate thread.
TEST_F(UnitTestError, UnblockReadUdp)
{
    Timeout timeout(std::chrono::seconds(3));

    // Read on a separate thread.
    UdpSocket receiver(TestGlobals::port);
    ASSERT_TRUE(receiver);
    auto task = std::async(std::launch::async, [&receiver]() -> bool {
        try
        {
            char buf[1];
            receiver.Read(buf, 1, nullptr, nullptr);
            std::cout << "Read did not fail as expected.\n"
                      << std::endl;
            return false;
        }
        catch (ProgramError const& e)
        {
            std::cout << "Read failed successfully:\n"
                      << e.what() << std::endl;
            return true;
        }
    });

    // Make sure the read is still blocking.
    ASSERT_EQ(task.wait_for(milliseconds(200)), std::future_status::timeout) << (receiver ? "Socket returned from read too early." : "Socket closed.");
    std::this_thread::sleep_for(milliseconds(200));
    receiver.Close();  // This call should block until the operation is done.
    ASSERT_FALSE(receiver);
    ASSERT_TRUE(task.get());
}

// Tests that closing the socket breaks a blocking read on a separate thread.
TEST_F(UnitTestError, UnblockReadUdpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    // Read on a separate thread.
    UdpSocket receiver(TestGlobals::port);
    ASSERT_TRUE(receiver);
    auto task = std::async(std::launch::async, [&receiver]() -> bool {
        char buf[1];
        ErrorCode ec;
        receiver.Read(buf, 1, nullptr, nullptr, &ec);
        try
        {
            if (ec)
                ec.Rethrow();
            std::cout << "Read did not fail as expected.\n"
                      << std::endl;
            return false;
        }
        catch (ProgramError const& e)
        {
            std::cout << "Read failed successfully:\n"
                      << e.what() << std::endl;
            return true;
        }
    });

    // Make sure the read is still blocking.
    ASSERT_EQ(task.wait_for(milliseconds(200)), std::future_status::timeout) << (receiver ? "Socket returned from read too early." : "Socket closed.");
    std::this_thread::sleep_for(milliseconds(200));
    receiver.Close();  // This call should block until the operation is done.
    ASSERT_FALSE(receiver);
    ASSERT_TRUE(task.get());
}

// Tests that closing the socket breaks a blocking accept on a separate thread.
TEST_F(UnitTestError, UnblockAccept)
{
    Timeout timeout(std::chrono::seconds(3));

    // Accept on a separate thread.
    TcpListener listener(TestGlobals::port);
    ASSERT_TRUE(listener);
    auto task = std::async(std::launch::async, [&listener]() -> bool {
        try
        {
            auto client = listener.Accept();
            std::cout << "Read did not fail as expected.\n"
                      << std::endl;
            return false;
        }
        catch (ProgramError const& e)
        {
            std::cout << "Accept failed successfully:\n"
                      << e.what() << std::endl;
            return true;
        }
    });

    // Make sure the read is still blocking.
    ASSERT_EQ(task.wait_for(milliseconds(200)), std::future_status::timeout) << (listener ? "Socket returned from read too early." : "Socket closed.");
    std::this_thread::sleep_for(milliseconds(200));
    listener.Close();  // This call should block until the operation is done.
    ASSERT_FALSE(listener);
    ASSERT_TRUE(task.get());
}

// Tests that closing the socket breaks a blocking accept on a separate thread.
TEST_F(UnitTestError, UnblockAcceptEc)
{
    Timeout timeout(std::chrono::seconds(3));

    // Accept on a separate thread.
    TcpListener listener(TestGlobals::port);
    ASSERT_TRUE(listener);
    auto task = std::async(std::launch::async, [&listener]() -> bool {
        ErrorCode ec;
        auto client = listener.Accept(&ec);
        try
        {
            if (ec)
                ec.Rethrow();
            std::cout << "Read did not fail as expected.\n"
                      << std::endl;
            return false;
        }
        catch (ProgramError const& e)
        {
            std::cout << "Accept failed successfully:\n"
                      << e.what() << std::endl;
            return true;
        }
    });

    // Make sure the accept is still blocking.
    ASSERT_EQ(task.wait_for(milliseconds(200)), std::future_status::timeout) << (listener ? "Socket returned from read too early." : "Socket closed.");
    std::this_thread::sleep_for(milliseconds(200));
    listener.Close();  // This call should block until the operation is done.
    ASSERT_FALSE(listener);
    ASSERT_TRUE(task.get());
}

}}}  // namespace strapper::net::test
