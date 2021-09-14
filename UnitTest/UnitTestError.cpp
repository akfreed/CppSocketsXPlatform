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

#include <TcpSocket.h>
#include <TcpSerializer.h>
#include <TcpListener.h>
#include <NetworkError.h>
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
TEST_F(UnitTestError, ReadTimeout)
{
    Timeout timeout(std::chrono::seconds(3));

    m_receiver.SetReadTimeout(1000);

    auto start = std::chrono::steady_clock::now();
    uint8_t buf[1];
    ASSERT_THROW(m_receiver.Read(buf, 1), NetworkError) << "Socket read did not throw.";
    auto stop = std::chrono::steady_clock::now();
    ASSERT_GT((stop - start), std::chrono::milliseconds(950)) << "Socket returned from read too early.";
    ASSERT_LT((stop - start), std::chrono::milliseconds(1050)) << "Socket returend from read too late.";

    ASSERT_FALSE(m_receiver.IsConnected()); // timing out should close the socket
}

// Tests that closing the socket breaks a blocking read on a separate thread.
// todo: Fix for windows by closing the socket before waiting.
TEST_F(UnitTestError, DISABLED_CloseFromOtherThread)
{
    Timeout timeout(std::chrono::seconds(3));

    std::atomic<bool> hasUnblocked = false;

    // Read on a separate thread.
    auto task = std::async(std::launch::async, [this, &hasUnblocked]() {
        char buf[1];
        m_receiver.Read(buf, 1);
        hasUnblocked = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Make sure the read is still blocking.
    ASSERT_FALSE(hasUnblocked) << (m_receiver.IsConnected() ? "Socket returned from read too early." : "Socket closed.");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    m_receiver.Close();  // This call should block until the operation is done.
    ASSERT_FALSE(m_receiver.IsConnected());
}

} } }
