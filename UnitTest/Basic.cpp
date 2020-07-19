// ==================================================================
// Copyright 2018 Alexander K. Freed
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

// Contains the basic tests.

#include "TcpListener.h"
#include "TcpSocket.h"
#include "UdpSocket.h"

#include <cstring>
#include <string>
#include <vector>
#include <array>

#include "TestReport.h"
#include "UnitTestMain.h"


//============================================================================
// tests

template <typename SOCKET>
TestReport SendRecvChar(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    TestReport report("SendRecvChar", "Sends and receives a char.");
    char sentData = 'f';

    if (!assumptions)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
        return report;
    }

    if (!IsValidSocket(sender))
    {
        report.ResultNotes = "Sender not connected.";
        return report;
    }
    if (!IsValidSocket(receiver))
    {
        report.ResultNotes = "Receiver not connected.";
        return report;
    }

    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    if (!sender.Write(sentData))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!receiver.SetReadTimeout(5000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    char c;
    if (!receiver.Read(c))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    if (c != sentData)
    {
        report.ResultNotes = "Read received wrong data.";
        return report;
    }

    report.Passed = true;
    return report;
}


//----------------------------------------------------------------------------

template <typename SOCKET>
TestReport SendRecvBool(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    TestReport report("SendRecvBool", "Sends and receives true and false.");

    if (!assumptions)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
        return report;
    }

    if (!IsValidSocket(sender))
    {
        report.ResultNotes = "Sender not connected.";
        return report;
    }
    if (!IsValidSocket(receiver))
    {
        report.ResultNotes = "Receiver not connected.";
        return report;
    }

    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    if (!sender.Write(false))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!sender.Write(true))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!sender.Write(true))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!receiver.SetReadTimeout(5000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    bool b;
    if (!receiver.Read(b))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    if (b != false)
    {
        report.ResultNotes = "Read received wrong data.";
        return report;
    }

    if (!receiver.Read(b))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    if (b != true)
    {
        report.ResultNotes = "Read received wrong data.";
        return report;
    }

    // this test is not necessary
    char c;
    if (!receiver.Read(c))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    if (c != 1)
    {
        report.ResultNotes = "Read received wrong data.";
        return report;
    }

    report.Passed = true;
    return report;
}


//----------------------------------------------------------------------------

template <typename SOCKET>
TestReport SendRecvInt(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    TestReport report("SendRecvInt", "Sends and receives an int32.");
    int sentData = -20;

    if (!assumptions)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
        return report;
    }

    if (!IsValidSocket(sender))
    {
        report.ResultNotes = "Sender not connected.";
        return report;
    }
    if (!IsValidSocket(receiver))
    {
        report.ResultNotes = "Receiver not connected.";
        return report;
    }

    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    if (!sender.Write(sentData))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!receiver.SetReadTimeout(5000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    int i;
    if (!receiver.Read(i))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    if (i != sentData)
    {
        report.ResultNotes = "Read received wrong data.";
        return report;
    }

    report.Passed = true;
    return report;
}


//----------------------------------------------------------------------------

template <typename SOCKET>
TestReport SendRecvDouble(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    TestReport report("SendRecvDouble", "Sends and receives a double.");
    double sentData = 5.1234567890;

    if (!assumptions)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
        return report;
    }

    if (!IsValidSocket(sender))
    {
        report.ResultNotes = "Sender not connected.";
        return report;
    }
    if (!IsValidSocket(receiver))
    {
        report.ResultNotes = "Receiver not connected.";
        return report;
    }

    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    if (!sender.Write(sentData))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!receiver.SetReadTimeout(5000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    double d;
    if (!receiver.Read(d))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    if (d != sentData)
    {
        report.ResultNotes = "Read received wrong data.";
        return report;
    }

    report.Passed = true;
    return report;
}


//----------------------------------------------------------------------------

template <typename SOCKET>
TestReport SendRecvBuf(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    TestReport report("SendRecvBuf", "Sends and receives a char buffer.");
    char sentData[6] = { 1, 2, 3, 4, 5, 6 };

    if (!assumptions)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
        return report;
    }

    if (!IsValidSocket(sender))
    {
        report.ResultNotes = "Sender not connected.";
        return report;
    }
    if (!IsValidSocket(receiver))
    {
        report.ResultNotes = "Receiver not connected.";
        return report;
    }

    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    if (!sender.Write(sentData, 5))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!receiver.SetReadTimeout(5000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    char buf[6] = { 0, 0, 0, 0, 0 };
    std::array<char, 6> expected = { 1, 2, 3, 4, 5, 0 };

    if (!receiver.Read(buf, 5))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    if (!bufferMatches(buf, expected, 5))
    {
        report.ResultNotes = "Read received wrong data.";
        return report;
    }

    if (!sender.Write(sentData + 3, 3))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!sender.Write(sentData, 3))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    expected = { 4, 5, 3, 4, 5, 0 };
    if (!receiver.Read(buf, 2))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    expected = { 6, 1, 2, 3, 5, 0 };
    if (!receiver.Read(buf, 4))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    report.Passed = true;
    return report;
}


//----------------------------------------------------------------------------

template <typename SOCKET>
TestReport SendRecvCharString(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    TestReport report("SendRecvCharString", "Sends and receives a char string.");
    char s[101] = "Hello, World!";

    if (!assumptions)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
        return report;
    }

    if (!IsValidSocket(sender))
    {
        report.ResultNotes = "Sender not connected.";
        return report;
    }
    if (!IsValidSocket(receiver))
    {
        report.ResultNotes = "Receiver not connected.";
        return report;
    }

    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    if (!sender.WriteString(s))
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!receiver.SetReadTimeout(5000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    char msg[101];
    memset(msg, 0xFF, 101);

    if (!receiver.ReadString(msg, 101))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    if (strcmp(s, msg) != 0)
    {
        report.ResultNotes = "Read received wrong data.";
        return report;
    }

    report.Passed = true;
    return report;
}


//============================================================================
// Test Basic Main

TestReport TestBasic()
{
    TestReport report("RunBasic", "Runs basic read/write tests.");

    TestReport result;
    TcpSocket senderTcp, receiverTcp;

    result = SelfConnect("11111", senderTcp, receiverTcp);
    report.SubTests.push_back(result);

    bool assumptions = result.Passed;

    result = SendRecvChar(assumptions, senderTcp, receiverTcp);
    report.SubTests.push_back(result);

    result = SendRecvBool(assumptions, senderTcp, receiverTcp);
    report.SubTests.push_back(result);

    result = SendRecvInt(assumptions, senderTcp, receiverTcp);
    report.SubTests.push_back(result);

    result = SendRecvDouble(assumptions, senderTcp, receiverTcp);
    report.SubTests.push_back(result);

    result = SendRecvBuf(assumptions, senderTcp, receiverTcp);
    report.SubTests.push_back(result);

    result = SendRecvCharString(assumptions, senderTcp, receiverTcp);
    report.SubTests.push_back(result);

    UdpSocket senderUdp, receiverUdp;
    result = SelfConnect(11111, senderUdp, receiverUdp);
    report.SubTests.push_back(result);
    assumptions = result.Passed;

    result = SendRecvBuf(assumptions, senderUdp, receiverUdp);
    report.SubTests.push_back(result);

    // set top-level report
    report.Passed = true;
    for (auto& subtest : report.SubTests)
    {
        if (!subtest.Passed)
        {
            report.Passed = false;
            break;
        }
    }
    
    return report;
}

