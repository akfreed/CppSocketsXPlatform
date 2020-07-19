//============================================================================
// Copyright (c) 2018 Alexander Freed
// 
// Contains the tests for byte ordering.
//============================================================================

#include "TcpListener.h"
#include "TcpSocket.h"
#include "UdpSocket.h"

#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <thread>
#include <atomic>
#include <chrono>

#include "TestReport.h"
#include "UnitTestMain.h"


//============================================================================
// tests

template <typename SOCKET>
static void read(SOCKET* toRead, std::atomic<bool>* hasClosed)
{
    bool b;
    toRead->Read(b);
    *hasClosed = true;
}


//----------------------------------------------------------------------------

template <typename SOCKET>
TestReport DataAvailable(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    TestReport report("DataAvailable", "Tests the DataAvailable() function.");

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

    if (!receiver.SetReadTimeout(2000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    int i = 5;
    if (!sender.Write(i))
    {
        report.ResultNotes = "Sender failed to write.";
        return report;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int available = receiver.DataAvailable();
    if (available == 0)
    {
        report.ResultNotes = "DataAvailable did not indicate data.";
        return report;
    }
    else if (available < 1)
    {
        report.ResultNotes = "DataAvailable error.";
        return report;
    }

    if (!receiver.Read(i))
    {
        report.ResultNotes = "Receiver read error.";
        return report;
    }

    available = receiver.DataAvailable();
    if (available > 0)
    {
        report.ResultNotes = "DataAvailable indicated data that did not exist.";
        return report;
    }
    else if (available < 0)
    {
        report.ResultNotes = "DataAvailable error.";
        return report;
    }

    report.Passed = true;
    return report;
}


//----------------------------------------------------------------------------

template <typename SOCKET>
static void write(SOCKET& sender)
{
    const int BUFLEN = 2;
    char buffer[BUFLEN];
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sender.Write(buffer, BUFLEN);
}


template <typename SOCKET>
TestReport DumpReadBuffer(bool assumptions, SOCKET& sender, SOCKET& receiver)
{
    using namespace std::chrono;

    TestReport report("DumpReadBuffer", "Tests the DumpReadBuffer() function.");

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

    if (!receiver.SetReadTimeout(2000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    if (receiver.DataAvailable() != 0)
    {
        report.ResultNotes = "Receiver had data in buffer before data was sent.";
        return report;
    }

    // Test 1, dump it all

    const int BUFLEN = 2000;
    char buffer[BUFLEN];
    if (!sender.Write(buffer, BUFLEN))
    {
        report.ResultNotes = "Sender failed to write.";
        return report;
    }

    std::this_thread::sleep_for(milliseconds(100));

    int available = receiver.DataAvailable();
    if (available == 0)
    {
        report.ResultNotes = "DataAvailable did not indicate data.";
        return report;
    }
    else if (available < 1)
    {
        report.ResultNotes = "DataAvailable error.";
        return report;
    }

    if (receiver.GetDumpCount() != 0)
    {
        report.ResultNotes = "DumpCount was not initialized to 0.";
        return report;
    }

    if (!receiver.DumpReadBuffer())
    {
        report.ResultNotes = "DumpReadBuffer error.";
        return report;
    }

    available = receiver.DataAvailable();
    if (available > 0)
    {
        report.ResultNotes = "DataAvailable indicated data that did not get dumped.";
        return report;
    }
    else if (available < 0)
    {
        report.ResultNotes = "DataAvailable error.";
        return report;
    }

    if (receiver.GetDumpCount() != 1)
    {
        report.ResultNotes = "DumpCount not incremented.";
        return report;
    }

    // Test 2, partial dumps
    if (!sender.Write(buffer, 2000))
    {
        report.ResultNotes = "Sender failed to write.";
        return report;
    }

    std::this_thread::sleep_for(milliseconds(100));

    available = receiver.DataAvailable();
    if (available == 0)
    {
        report.ResultNotes = "DataAvailable did not indicate data.";
        return report;
    }
    else if (available < 1)
    {
        report.ResultNotes = "DataAvailable error.";
        return report;
    }

    if (!receiver.DumpReadBuffer(500))
    {
        report.ResultNotes = "DumpReadBuffer error.";
        return report;
    }

    if (!receiver.DumpReadBuffer(1499))
    {
        report.ResultNotes = "DumpReadBuffer error.";
        return report;
    }

    if (receiver.DataAvailable() <= 0)
    {
        report.ResultNotes = "DataAvailable did not indicate data.";
        return report;
    }

    if (!receiver.DumpReadBuffer(1))
    {
        report.ResultNotes = "DumpReadBuffer error.";
        return report;
    }

    available = receiver.DataAvailable();
    if (available > 0)
    {
        report.ResultNotes = "DataAvailable indicated data that did not get dumped.";
        return report;
    }
    else if (available < 0)
    {
        report.ResultNotes = "DataAvailable error.";
        return report;
    }

    if (receiver.GetDumpCount() != 4)
    {
        report.ResultNotes = "DumpCount was not incremented.";
        return report;
    }

    // Test 3, dump blocks until all data is sent
    if (!sender.Write(buffer, 4))
    {
        report.ResultNotes = "Sender failed to write.";
        return report;
    }

    std::thread t(write<SOCKET>, std::ref(sender));

    auto start = high_resolution_clock::now();
    if (!receiver.DumpReadBuffer(5))
    {
        report.ResultNotes = "DumpReadBuffer error.";
        t.join();
        return report;
    }
    auto time = high_resolution_clock::now() - start;
    t.join();

    if (time < milliseconds(975))
    {
        report.ResultNotes = "DumpReadBuffer did not block.";
        return report;
    }
    else if (time > milliseconds(1075))
    {
        report.ResultNotes = "DumpReadBuffer took too long.";
        return report;
    }

    if (receiver.DataAvailable() != 1)
    {
        report.ResultNotes = "DumpReadBuffer read too much.";
        return report;
    }

    report.Passed = true;
    return report;
}


//----------------------------------------------------------------------------

TestReport ReadTimeout(bool assumptions=true)
{
    TestReport report("ReadTimeout", "Tests that SetReadTimeout breaks a blocking read and closes the socket.");

    if (!assumptions)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
        return report;
    }

    TcpSocket sender, receiver;
    TestReport connected = SelfConnect("11112", sender, receiver);
    report.SubTests.push_back(connected);
    if (!connected.Passed)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
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

    if (!receiver.SetReadTimeout(2000))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    // move managed objects to unmanaged objects in case of unrecoverable failure.
    TcpSocket* pReceiver = new TcpSocket(std::move(receiver));
    std::atomic<bool>* hasClosed = new std::atomic<bool>(false);

    // read on separate thread
    std::thread t(read<TcpSocket>, pReceiver, hasClosed);

    // sleep for 3 seconds total, but check after 1 to make sure the read is still blocking
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (*hasClosed)
    {
        if (IsValidSocket(*pReceiver))
            report.ResultNotes = "Socket unexpectedly returned from read too early.";
        else
            report.ResultNotes = "Socket unexpectedly closed.";
        t.join();
        delete pReceiver;
        delete hasClosed;
        return report;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    bool delinquent = false;

    // other thread should have terminated by now.
    if (!(*hasClosed))
    {
        report.ResultNotes = "Socket read did not time-out. Had to close socket.";

        // try closing the sockets
        sender.Close();
        pReceiver->Close();

        if (!(*hasClosed))
        {
            report.ResultNotes = "Socket read did not time-out, even after closing socket!";
            delinquent = true;
        }
    }
    else
        report.Passed = true;


    if (report.Passed)
    {
        // timing out should close the socket
        if (IsValidSocket(*pReceiver))
        {
            report.ResultNotes = "Receiver did not close after timing out.";
            report.Passed = false;
        }
    }

    if (!delinquent)
    {
        // clean up
        t.join();
        delete hasClosed;
        delete pReceiver;
    }
    else
    {
        t.detach();
        g_mustTerminate = true;
    }

    return report;
}


//----------------------------------------------------------------------------

TestReport CloseFromOtherThread()
{
    TestReport report("CloseFromOtherThread", "Tests that closing the socket breaks a blocking read on a separate thread.");

    TcpSocket sender, receiver;
    TestReport connected = SelfConnect("11113", sender, receiver);
    report.SubTests.push_back(connected);
    if (!connected.Passed)
    {
        report.ResultNotes = "Failed assumptions.";
        report.FailedAssumptions = true;
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

    if (!receiver.SetReadTimeout(300))
    {
        report.ResultNotes = "Receiver setsockopt failed when setting read timeout.";
        return report;
    }

    // move managed objects to unmanaged objects in case of unrecoverable failure.
    TcpSocket* pReceiver = new TcpSocket(std::move(receiver));
    std::atomic<bool>* hasClosed = new std::atomic<bool>(false);

    // read on separate thread
    std::thread t(read<TcpSocket>, pReceiver, hasClosed);

    // sleep for 200ms total, then close the socket
    // sleep for 100ms and check to make sure the read is still going on
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (*hasClosed)
    {
        if (IsValidSocket(*pReceiver))
            report.ResultNotes = "Socket unexpectedly returned from read too early.";
        else
            report.ResultNotes = "Socket unexpectedly closed.";
        t.join();
        delete pReceiver;
        delete hasClosed;
        return report;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pReceiver->Close();

    bool delinquent = false;

    // Close should block until the operation is done
    if (pReceiver->IsConnected())
    {
        report.ResultNotes = "Socket read did break on close. Had to wait for read time-out.";

        // try closing the sender socket and wait for read timeout to kick in.
        sender.Close();

        // sleep for another 2 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (!(*hasClosed))
        {
            report.ResultNotes = "Socket is still blocking, even after timeout!";
            delinquent = true;
        }
    }
    else
        report.Passed = true;

    if (!delinquent)
    {
        // clean up
        t.join();
        delete hasClosed;
        delete pReceiver;
    }
    else
    {
        t.detach();
        g_mustTerminate = true;
    }

    return report;
}


//============================================================================
// Test Error Main

TestReport TestError()
{
    TestReport report("TestError", "Tests handling of various errors.");

    TestReport result;
    TcpSocket sender, receiver;

    result = SelfConnect("11111", sender, receiver);
    report.SubTests.push_back(result);

    bool assumptions = result.Passed;

    result = DataAvailable(assumptions, sender, receiver);
    report.SubTests.push_back(result);

    result = DumpReadBuffer(assumptions, sender, receiver);
    report.SubTests.push_back(result);

    //// skipping test because it takes too long. But it's a good test.
    //result = ReadTimeout(false);
    //result.Passed = true;
    //result.Skipped = true;
    //result.ResultNotes = "Test skipped because it takes too long.";
    result = ReadTimeout();
    report.SubTests.push_back(result);

    result = CloseFromOtherThread();
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


