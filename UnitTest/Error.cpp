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

#include "TestGlobals.h"
#include "Timeout.h"
#include "TcpSocket.h"
#include "TcpSerializer.h"
#include "TcpListener.h"

#include <thread>
#include <chrono>
#include <future>
#include <atomic>

class Error : public ::testing::Test
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
        TcpListener listener(TestGlobals::portString);
        ASSERT_TRUE(listener.IsValid());
        ASSERT_TRUE(m_sender.Connect(TestGlobals::localhost, TestGlobals::portString));
        ASSERT_TRUE(m_sender.IsConnected());
        m_receiver = listener.Accept();
        ASSERT_TRUE(m_receiver.IsConnected());
        ASSERT_EQ(m_receiver.DataAvailable(), 0);
    }

    TcpSocket m_sender;
    TcpSocket m_receiver;
};

// Tests that SetReadTimeout breaks a blocking read and closes the socket.
TEST_F(Error, ReadTimeout)
{
    Timeout timeout(std::chrono::seconds(3));

    ASSERT_TRUE(m_receiver.SetReadTimeout(1000));

    std::atomic<bool> hasUnblocked = false;

    // Read on a separate thread.
    auto task = std::async(std::launch::async, [this, &hasUnblocked]() {
        char buf[1];
        m_receiver.Read(buf, 1);
        hasUnblocked = true;
    });

    // Check after half a second to make sure the read is still blocking.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_FALSE(hasUnblocked) << (m_receiver.IsConnected() ? "Socket returned from read too early." : "Socket closed.");

    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    if (!hasUnblocked)
    {
        // Try closing the sockets.
        m_sender.Close();
        m_receiver.Close();
        ASSERT_TRUE(hasUnblocked);
        ASSERT_TRUE(false);
    }

    ASSERT_FALSE(m_receiver.IsConnected()); // timing out should close the socket
}

// Tests that closing the socket breaks a blocking read on a separate thread.
// todo: Fix for windows by closing the socket before waiting.
TEST_F(Error, DISABLED_CloseFromOtherThread)
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
