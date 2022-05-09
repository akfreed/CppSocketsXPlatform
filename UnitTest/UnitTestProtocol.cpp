// ==================================================================
// Copyright 2022 Alexander K. Freed
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
#include <strapper/net/TcpListener.h>
#include <strapper/net/UdpSocket.h>
#include <strapper/net/SocketError.h>
#include "TestGlobals.h"
#include "Timeout.h"

#include <thread>
#include <chrono>
#include <future>
#include <atomic>
#include <exception>

namespace strapper { namespace net { namespace test {

using std::chrono::milliseconds;

class UnitTestProtocol : public ::testing::Test
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

TEST_F(UnitTestProtocol, ShutdownSendTcp)
{
    Timeout timeout(std::chrono::seconds(3));

    int64_t const toWrite = 0xA1B2C3D45E6F809D;
    m_sender.Write(&toWrite, sizeof(toWrite));
    ASSERT_TRUE(m_sender);
    m_sender.ShutdownSend();
    ASSERT_TRUE(m_sender);
    int64_t toRead = 0;
    static_assert(sizeof(toWrite) == sizeof(toRead), "Buffers must be the same size.");
    ASSERT_EQ(m_receiver.DataAvailable(), sizeof(toWrite));
    ASSERT_TRUE(m_receiver.Read(&toRead, sizeof(toRead)));
    ASSERT_EQ(toWrite, toRead);

    uint8_t c = 0xAF;
    ASSERT_EQ(m_receiver.DataAvailable(), 0);
    ASSERT_FALSE(m_receiver.Read(&c, sizeof(c)));
    ASSERT_TRUE(m_receiver);
    ASSERT_EQ(c, 0xAF);

    std::atomic<bool> ready = false;
    auto task = std::async(std::launch::async, [this, &ready]() {
        ready = true;
        std::this_thread::sleep_for(milliseconds(200));
        m_receiver.ShutdownSend();
    });

    while (!ready)
        ;
    auto start = std::chrono::steady_clock::now();
    ASSERT_EQ(m_sender.DataAvailable(), 0);
    std::this_thread::sleep_for(milliseconds(100));
    ASSERT_EQ(m_sender.DataAvailable(), 0);
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c)));
    auto stop = std::chrono::steady_clock::now();
    ASSERT_TRUE(m_sender);
    ASSERT_EQ(c, 0xAF);
    ASSERT_TRUE(m_receiver);
    auto diff = stop - start;
    ASSERT_GT(diff, milliseconds(150));
    ASSERT_LT(diff, milliseconds(250));
    ASSERT_NO_THROW(task.get());
}

TEST_F(UnitTestProtocol, ShutdownSendTcpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    ErrorCode ec;
    int64_t const toWrite = 0xA1B2C3D45E6F809D;
    m_sender.Write(&toWrite, sizeof(toWrite), &ec);
    ASSERT_TRUE(m_sender);
    ASSERT_FALSE(ec);
    m_sender.ShutdownSend(&ec);
    ASSERT_TRUE(m_sender);
    ASSERT_FALSE(ec);
    int64_t toRead = 0;
    static_assert(sizeof(toWrite) == sizeof(toRead), "Buffers must be the same size.");
    ASSERT_EQ(m_receiver.DataAvailable(&ec), sizeof(toWrite));
    ASSERT_FALSE(ec);
    ASSERT_TRUE(m_receiver.Read(&toRead, sizeof(toRead), &ec));
    ASSERT_EQ(toWrite, toRead);
    ASSERT_FALSE(ec);

    uint8_t c = 0xAF;
    ASSERT_EQ(m_receiver.DataAvailable(&ec), 0);
    ASSERT_FALSE(ec);
    ASSERT_FALSE(m_receiver.Read(&c, sizeof(c), &ec));
    ASSERT_TRUE(m_receiver);
    ASSERT_FALSE(ec);
    ASSERT_EQ(c, 0xAF);

    std::atomic<bool> ready = false;
    auto task = std::async(std::launch::async, [this, &ready]() -> ErrorCode {
        ErrorCode ec;
        ready = true;
        std::this_thread::sleep_for(milliseconds(200));
        m_receiver.ShutdownSend(&ec);
        return ec;
    });

    while (!ready)
        ;
    auto start = std::chrono::steady_clock::now();
    ASSERT_EQ(m_sender.DataAvailable(&ec), 0);
    ASSERT_FALSE(ec);
    std::this_thread::sleep_for(milliseconds(100));
    ASSERT_EQ(m_sender.DataAvailable(&ec), 0);
    ASSERT_FALSE(ec);
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), &ec));
    auto stop = std::chrono::steady_clock::now();
    ASSERT_FALSE(ec);
    ASSERT_TRUE(m_sender);
    ASSERT_EQ(c, 0xAF);
    ASSERT_TRUE(m_receiver);
    auto diff = stop - start;
    ASSERT_GT(diff, milliseconds(150));
    ASSERT_LT(diff, milliseconds(250));
    ec = task.get();
    ASSERT_FALSE(ec);
}

TEST_F(UnitTestProtocol, ReadAfterShutdownTcp)
{
    Timeout timeout(std::chrono::seconds(3));

    int64_t const toWrite = 0xA1B2C3D45E6F809D;
    m_sender.Write(&toWrite, sizeof(toWrite));
    ASSERT_TRUE(m_sender);
    m_sender.ShutdownSend();
    ASSERT_TRUE(m_sender);
    int64_t toRead = 0;
    static_assert(sizeof(toWrite) == sizeof(toRead), "Buffers must be the same size.");
    ASSERT_EQ(m_receiver.DataAvailable(), sizeof(toWrite));
    ASSERT_TRUE(m_receiver.Read(&toRead, sizeof(toRead)));
    ASSERT_EQ(toWrite, toRead);

    uint8_t c = 0xAF;
    ASSERT_EQ(m_receiver.DataAvailable(), 0);
    ASSERT_FALSE(m_receiver.Read(&c, sizeof(c)));
    ASSERT_TRUE(m_receiver);
    ASSERT_EQ(c, 0xAF);

    // Read should throw this time.
    ASSERT_EQ(m_receiver.DataAvailable(), 0);
    ASSERT_THROW(m_receiver.Read(&c, sizeof(c)), SocketError);
    ASSERT_FALSE(m_receiver);
    ASSERT_EQ(c, 0xAF);

    // And throw again.
    ASSERT_THROW(m_receiver.DataAvailable(), ProgramError);
    ASSERT_THROW(m_receiver.Read(&c, sizeof(c)), ProgramError);
    ASSERT_FALSE(m_receiver);
    ASSERT_EQ(c, 0xAF);

    // Check sender
    ASSERT_TRUE(m_sender);

    ASSERT_EQ(m_sender.DataAvailable(), 0);
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c)));
    ASSERT_TRUE(m_sender);
    ASSERT_EQ(c, 0xAF);

    // Read should throw this time.
    ASSERT_EQ(m_sender.DataAvailable(), 0);
    ASSERT_THROW(m_sender.Read(&c, sizeof(c)), SocketError);
    ASSERT_FALSE(m_sender);
    ASSERT_EQ(c, 0xAF);

    // And throw again.
    ASSERT_THROW(m_sender.DataAvailable(), ProgramError);
    ASSERT_THROW(m_sender.Read(&c, sizeof(c)), ProgramError);
    ASSERT_FALSE(m_sender);
    ASSERT_EQ(c, 0xAF);
}

TEST_F(UnitTestProtocol, ReadAfterShutdownTcpEc)
{
    Timeout timeout(std::chrono::seconds(3));

    ErrorCode ec;
    int64_t const toWrite = 0xA1B2C3D45E6F809D;
    m_sender.Write(&toWrite, sizeof(toWrite), &ec);
    ASSERT_TRUE(m_sender);
    ASSERT_FALSE(ec);
    m_sender.ShutdownSend(&ec);
    ASSERT_TRUE(m_sender);
    ASSERT_FALSE(ec);
    int64_t toRead = 0;
    static_assert(sizeof(toWrite) == sizeof(toRead), "Buffers must be the same size.");
    ASSERT_EQ(m_receiver.DataAvailable(&ec), sizeof(toWrite));
    ASSERT_FALSE(ec);
    ASSERT_TRUE(m_receiver.Read(&toRead, sizeof(toRead), &ec));
    ASSERT_FALSE(ec);
    ASSERT_EQ(toWrite, toRead);

    uint8_t c = 0xAF;
    ASSERT_EQ(m_receiver.DataAvailable(&ec), 0);
    ASSERT_FALSE(ec);
    ASSERT_FALSE(m_receiver.Read(&c, sizeof(c), &ec));
    ASSERT_FALSE(ec);
    ASSERT_TRUE(m_receiver);
    ASSERT_EQ(c, 0xAF);

    // Read should fail this time.
    ASSERT_EQ(m_receiver.DataAvailable(&ec), 0);
    ASSERT_FALSE(ec);
    ASSERT_FALSE(m_receiver.Read(&c, sizeof(c), &ec));
    ASSERT_TRUE(ec);
    ASSERT_THROW(ec.Rethrow(), SocketError);
    ec = ErrorCode(); // Reset ec.
    ASSERT_FALSE(m_receiver);
    ASSERT_EQ(c, 0xAF);

    // And fail again.
    ASSERT_EQ(m_receiver.DataAvailable(&ec), 0);
    ASSERT_TRUE(ec);
    ASSERT_THROW(ec.Rethrow(), ProgramError);
    ec = ErrorCode();
    ASSERT_FALSE(m_receiver.Read(&c, sizeof(c), &ec));
    ASSERT_TRUE(ec);
    ASSERT_THROW(ec.Rethrow(), ProgramError);
    ec = ErrorCode();
    ASSERT_FALSE(m_receiver);
    ASSERT_EQ(c, 0xAF);

    // Check sender
    ASSERT_TRUE(m_sender);

    ASSERT_EQ(m_sender.DataAvailable(&ec), 0);
    ASSERT_FALSE(ec);
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), &ec));
    ASSERT_FALSE(ec);
    ASSERT_TRUE(m_sender);
    ASSERT_EQ(c, 0xAF);

    // Read should fail this time.
    ASSERT_EQ(m_sender.DataAvailable(&ec), 0);
    ASSERT_FALSE(ec);
    // Note slight difference from previous tests. Test here with a null EC.
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), nullptr));
    ASSERT_FALSE(m_sender);
    ASSERT_EQ(c, 0xAF);

    // And fail again.
    ASSERT_EQ(m_sender.DataAvailable(&ec), 0);
    ASSERT_TRUE(ec);
    ASSERT_THROW(ec.Rethrow(), ProgramError);
    ec = ErrorCode();
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), &ec));
    ASSERT_TRUE(ec);
    ASSERT_THROW(ec.Rethrow(), ProgramError);
    ec = ErrorCode();
    ASSERT_FALSE(m_sender);
    ASSERT_EQ(c, 0xAF);

    ASSERT_EQ(m_sender.DataAvailable(nullptr), 0);
    ASSERT_EQ(m_sender.DataAvailable(nullptr), 0);
    ASSERT_EQ(m_sender.DataAvailable(nullptr), 0);
    ASSERT_EQ(m_sender.DataAvailable(nullptr), 0);
    ASSERT_EQ(m_sender.DataAvailable(nullptr), 0);
    ASSERT_EQ(m_sender.DataAvailable(&ec), 0);
    ASSERT_TRUE(ec);
    ASSERT_THROW(ec.Rethrow(), ProgramError);
    ec = ErrorCode();

    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), nullptr));
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), nullptr));
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), nullptr));
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), nullptr));
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), nullptr));
    ASSERT_FALSE(m_sender.Read(&c, sizeof(c), &ec));
    ASSERT_TRUE(ec);
    ASSERT_THROW(ec.Rethrow(), ProgramError);
}

} } }
