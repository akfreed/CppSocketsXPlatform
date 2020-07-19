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

// Contains the tests for byte ordering.

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
TestReport EndianCheckInt(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    TestReport report("EndianCheckInt", "Sends and receives int32s, checking endianness.");

    if (!assumptions)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
        return report;
    }

    static_assert(sizeof(int) == 4 && sizeof(char) == 1, "System does not have standard POD sizes.");

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
    
    int value = 0x3CABBA6E;
    char asBuffer[4];

    memcpy(asBuffer, &value, 4);

    if (asBuffer[0] == 0x6E)
    {
        report.ResultNotes = "This machine is little-endian.";
    }
    else if (asBuffer[0] == 0x3C)
    {
        report.ResultNotes = "This machine is big-endian.";
    }
    else if (asBuffer[0] == static_cast<char>(static_cast<unsigned char>(0xAB)) && asBuffer[2] == 0x6E)
    {
        report.ResultNotes = "This machine is middle-endian.";
    }
    else
    {
        report.ResultNotes = "This machine has unknown endianness.";
    }

    
    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    if (!sender.Write(value) || !sender.Write(value))  // write twice
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!receiver.SetReadTimeout(5000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    char readBuffer[4];
    if (!receiver.Read(readBuffer, 4))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    // readBuffer should be big-endian
    std::array<char, 4> expected = {
        static_cast<char>(static_cast<unsigned char>(0x3C)),
        static_cast<char>(static_cast<unsigned char>(0xAB)),
        static_cast<char>(static_cast<unsigned char>(0xBA)),
        static_cast<char>(static_cast<unsigned char>(0x6E))
    };

    if (!bufferMatches(readBuffer, expected, 4))
    {
        report.ResultNotes = "Read buffer was not big-endian.";
        return report;
    }

    int readInt;
    if (!receiver.Read(readInt))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    memcpy(readBuffer, &readInt, 4);

    if (!bufferMatches(readBuffer, asBuffer, 4))
    {
        report.ResultNotes = "Converted buffer did not match sent buffer.";
        return report;
    }

    report.Passed = true;
    return report;
}


//----------------------------------------------------------------------------

template <typename SOCKET>
TestReport EndianCheckDouble(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    TestReport report("EndianCheckDouble", "Sends and receives doubles, checking endianness.");

    if (!assumptions)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
        return report;
    }

    static_assert(sizeof(double) == 8 && sizeof(unsigned long long) == 8 && sizeof(char) == 1, "System does not have standard POD sizes.");

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

    unsigned long long value = 0x0807060504030201;
    char asBuffer[8];

    memcpy(asBuffer, &value, sizeof(value));

    if (asBuffer[0] == 0x01)
    {
        report.ResultNotes = "This machine is little-endian.";
    }
    else if (asBuffer[0] == 0x08)
    {
        report.ResultNotes = "This machine is big-endian.";
    }
    else if (asBuffer[0] == char(0x07) && asBuffer[2] == char(0x05))
    {
        report.ResultNotes = "This machine is middle-endian...I think...";
    }
    else
    {
        report.ResultNotes = "This machine has unknown endianness.";
    }
    
    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    double toSend;
    memcpy(&toSend, &value, sizeof(value));

    if (!sender.Write(toSend) || !sender.Write(toSend))  // write twice
    {
        report.ResultNotes = "Write failed.";
        return report;
    }

    if (!receiver.SetReadTimeout(5000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    char readBuffer[8];
    if (!receiver.Read(readBuffer, 8))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    // readBuffer should be big-endian
    std::array<char, 8> expected = { 8, 7, 6, 5, 4, 3, 2, 1 };

    if (!bufferMatches(readBuffer, expected, expected.size()))
    {
        report.ResultNotes = "Read buffer was not big-endian.";
        return report;
    }

    double readDouble;
    if (!receiver.Read(readDouble))
    {
        report.ResultNotes = "Read failed.";
        return report;
    }

    memcpy(readBuffer, &readDouble, sizeof(readDouble));

    if (!bufferMatches(readBuffer, asBuffer, sizeof(readDouble)))
    {
        report.ResultNotes = "Converted buffer did not match sent buffer.";
        return report;
    }

    report.Passed = true;
    return report;
}


//============================================================================
// Test Endian Main

TestReport TestEndian()
{
    TestReport report("TestEndian", "Tests the byte order swapping of the host.");

    TestReport result;
    TcpSocket sender, receiver;

    result = SelfConnect("11111", sender, receiver);
    report.SubTests.push_back(result);

    bool assumptions = result.Passed;

    result = EndianCheckInt(assumptions, sender, receiver);
    report.SubTests.push_back(result);

    result = EndianCheckDouble(assumptions, sender, receiver);
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


