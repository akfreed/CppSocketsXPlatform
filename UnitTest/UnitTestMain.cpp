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

// Contains main and some non-templated utility functions.

#include "TcpListener.h"
#include "TcpSocket.h"
#include "UdpSocket.h"

#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <iostream>

#include "TestReport.h"

#include "UnitTestMain.h"


//============================================================================
// prototypes

extern TestReport TestBasic();
extern TestReport TestEndian();
extern TestReport TestError();


//============================================================================
// globals

bool g_mustTerminate = false;
    // The main thread must call std::terminate() instead of return
    // This is used when a thread could not be joined, but the program could continue.
    // For example, a thread blocked on read and never returned.


//============================================================================
// non-templated utility functions 

TestReport SelfConnect(const char* port, TcpSocket& clientOut, TcpSocket& hostOut)
{
    std::string portString(port);
    std::string name = "SelfConnect (port " + portString + ")";
    TestReport report(std::move(name), "Creates a TCP connection to 127.0.0.1. Starts a listener, connects/accepts, then closes the listener.");

    TcpListener listener(port);
    if (!listener.IsValid())
    {
        report.ResultNotes = "Unable to start listener.";
        return report;
    }

    TcpSocket newClient("127.0.0.1", port);
    if (!newClient.IsConnected())
    {
        report.ResultNotes = "Unable to connect client to listener.";
        return report;
    }

    TcpSocket newHost = listener.Accept();
    if (!newClient.IsConnected())
    {
        report.ResultNotes = "Accept error.";
        return report;
    }

    clientOut = std::move(newClient);
    hostOut = std::move(newHost);

    report.Passed = true;
    return report;
}


TestReport SelfConnect(unsigned short port, UdpSocket& clientOut, UdpSocket& hostOut)
{
    std::string portString = std::to_string(port);
    std::string name = "SelfConnect (port " + portString + ")";
    TestReport report(std::move(name), "Opens 2 UDP sockets. One is bound to the given port. Both point to 127.0.0.1.");

    clientOut = UdpSocket("127.0.0.1", port);
    if (!IsValidSocket(clientOut))
    {
        report.ResultNotes = "Unable to open host socket.";
        return report;
    }

    hostOut = UdpSocket(port);
    if (!IsValidSocket(hostOut))
    {
        report.ResultNotes = "Unable to open client socket.";
        return report;
    }

    report.Passed = true;
    return report;
}


// These 3 functions allow a unifying function call API even though the functions are named differently for UdpSockets
bool IsValidSocket(TcpSocket& socket)
{
    return socket.IsConnected();
}


bool IsValidSocket(TcpListener& listener)
{
    return listener.IsValid();
}


bool IsValidSocket(UdpSocket& socket)
{
    return socket.IsValid();
}


//----------------------------------------------------------------------------

static void DisplayResults(const TestReport& report, std::string tabbing="")
{
    std::string message;

    if (report.Skipped || report.FailedAssumptions)
        message = "[SKIP] " + report.Name;
    else if (report.Passed)
        message = "[ OK ] " + report.Name;
    else
        message = "[FAIL] " + report.Name;

    if (report.ResultNotes != "")
        message += " - " + report.ResultNotes;

    std::cout << tabbing << message << std::endl;

    // recursive call
    for (auto subtest : report.SubTests)
        DisplayResults(subtest, tabbing + "  ");
}


//============================================================================
// Test Main

static TestReport TestMain()
{
    TestReport top("Socket Test Suite", "");

    WinsockContextManager wcm;
    if (!wcm.IsInitialized())
    {
        top.ResultNotes = "Unable to initialize winsock.\n";
        return top;
    }

    top.SubTests.push_back(TestBasic());
    top.SubTests.push_back(TestEndian());
    top.SubTests.push_back(TestError());

    top.Passed = true;
    for (auto& subtest : top.SubTests)
    {
        if (!subtest.Passed)
        {
            top.Passed = false;
            break;
        }
    }

    return top;
}


//============================================================================
// main

int main()
{
    std::cout << "Running Tests..." << std::endl;

    TestReport report = TestMain();
    DisplayResults(report);

    if (g_mustTerminate)
        std::terminate();
    else
        return EXIT_SUCCESS;
}
