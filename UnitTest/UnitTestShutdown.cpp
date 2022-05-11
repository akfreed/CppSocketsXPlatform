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

#include <strapper/net/TcpBasicSocket.h>
#include <strapper/net/TcpBasicListener.h>
#include "TestGlobals.h"
#include "Timeout.h"

namespace strapper { namespace net { namespace test {

using std::chrono::milliseconds;

class UnitTestShutdown : public ::testing::Test
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
        TcpBasicListener listener(TestGlobals::port);
        ASSERT_TRUE(listener);
        m_sender = TcpBasicSocket(TestGlobals::localhost, TestGlobals::port);
        ASSERT_TRUE(m_sender);
        m_receiver = listener.Accept();
        ASSERT_TRUE(m_receiver.IsConnected());
        ASSERT_EQ(m_receiver.DataAvailable(), 0u);
    }

    TcpBasicSocket m_sender;
    TcpBasicSocket m_receiver;
};

TEST_F(UnitTestShutdown, ABXY)
{
    m_sender.ShutdownSend();
    m_receiver.ShutdownReceive();
    m_receiver.ShutdownSend();
    ASSERT_NO_THROW(m_sender.ShutdownReceive());
}

TEST_F(UnitTestShutdown, AXBY)
{
    m_sender.ShutdownSend();
    m_receiver.ShutdownSend();
    ASSERT_NO_THROW(m_receiver.ShutdownReceive());
    ASSERT_NO_THROW(m_sender.ShutdownReceive());
}

TEST_F(UnitTestShutdown, AXYB)
{
    m_sender.ShutdownSend();
    m_receiver.ShutdownSend();
    ASSERT_NO_THROW(m_sender.ShutdownReceive());
    ASSERT_NO_THROW(m_receiver.ShutdownReceive());
}

TEST_F(UnitTestShutdown, AYXB)
{
    m_sender.ShutdownSend();
    m_sender.ShutdownReceive();
    m_receiver.ShutdownSend();
    ASSERT_NO_THROW(m_receiver.ShutdownReceive());
}

TEST_F(UnitTestShutdown, YAXB)
{
    m_sender.ShutdownReceive();
    m_sender.ShutdownSend();
    m_receiver.ShutdownSend();
    ASSERT_NO_THROW(m_receiver.ShutdownReceive());
}

TEST_F(UnitTestShutdown, YXAB)
{
    m_sender.ShutdownReceive();
    m_receiver.ShutdownSend();
    m_sender.ShutdownSend();
    ASSERT_NO_THROW(m_receiver.ShutdownReceive());
}

TEST_F(UnitTestShutdown, ABYX)
{
    m_sender.ShutdownSend();
    m_receiver.ShutdownReceive();
    m_sender.ShutdownReceive();
    m_receiver.ShutdownSend();
}

TEST_F(UnitTestShutdown, AYBX)
{
    m_sender.ShutdownSend();
    m_sender.ShutdownReceive();
    m_receiver.ShutdownReceive();
    m_receiver.ShutdownSend();
}

TEST_F(UnitTestShutdown, YABX)
{
    m_sender.ShutdownReceive();
    m_sender.ShutdownSend();
    m_receiver.ShutdownReceive();
    m_receiver.ShutdownSend();
}

TEST_F(UnitTestShutdown, YBAX)
{
    m_sender.ShutdownReceive();
    m_receiver.ShutdownReceive();
    m_sender.ShutdownSend();
    m_receiver.ShutdownSend();
}

TEST_F(UnitTestShutdown, YBXA)
{
    m_sender.ShutdownReceive();
    m_receiver.ShutdownReceive();
    m_receiver.ShutdownSend();
    m_sender.ShutdownSend();
}

TEST_F(UnitTestShutdown, YXBA)
{
    m_sender.ShutdownReceive();
    m_receiver.ShutdownSend();
    m_receiver.ShutdownReceive();
    m_sender.ShutdownSend();
}

} } }
