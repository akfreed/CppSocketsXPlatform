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

// Contains some templated utility functions.
// Also contains the prototypes for non-templated utility functions.

#pragma once

#include "TcpListener.h"
#include "TcpSocket.h"
#include "UdpSocket.h"

#include "TestReport.h"


//============================================================================
// globals

extern bool g_mustTerminate;
    // The main thread must call std::terminate() instead of return
    // This is used when a thread could not be joined, but the program could continue.
    // For example, a thread blocked on read and never returned.


//============================================================================
// non-templated utility function prototypes

TestReport SelfConnect(const char* port, TcpSocket& clientOut, TcpSocket& hostOut);
TestReport SelfConnect(unsigned short port, UdpSocket& clientOut, UdpSocket& hostOut);

// These 3 functions allow a unifying function call API even though the functions are named differently for UdpSockets
bool IsValidSocket(TcpSocket& socket);
bool IsValidSocket(TcpListener& listener);
bool IsValidSocket(UdpSocket& socket);


//============================================================================
// templated utility functions

template <typename INDEXABLE_1, typename INDEXABLE_2>
bool bufferMatches(INDEXABLE_1&& l, INDEXABLE_2&& r, unsigned len)
{
    bool match = true;
    for (unsigned i = 0; i < len; ++i)
        if (l[i] != r[i])
            match = false;
    return match;
}
