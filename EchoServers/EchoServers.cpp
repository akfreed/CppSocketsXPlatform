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

// Contains the code for TCP and UDP echo servers. These are pretty simple.
// They are one function each.

#include "TcpListener.h"
#include "TcpSocket.h"
#include "UdpSocket.h"

#include <cstring>

#include "EchoServers.h"


// return 0 for success
// return 1 for listener error
// return 2 for accept error
// return 3 for unexpected close
int TcpEchoServer(const char* port)
{
    TcpListener listener(port);
    if (!listener.IsValid())
        return 1;

    TcpSocket client = listener.Accept();
    listener.Close();
    if (!client.IsConnected())
        return 2;

    char buf[1000];
    buf[0] = '\0';

    while (strncmp(buf, "exit", 5) != 0)
    {
        if (client.ReadString(buf, 1000) <= 0)
            return 3;
        client.WriteString(buf);
    }

    return 0;
}


// return 0 for success
// return 1 for socket error
// return 2 for unexpected close
int UdpEchoServer(unsigned short port)
{
    UdpSocket client(port);
    if (!client.IsValid())
        return 1;

    int amountRead;
    char buf[1000];
    buf[0] = '\0';

    while (strncmp(buf, "exit", 5) != 0)
    {
        amountRead = client.Read(buf, 1000);
        if (amountRead < 0)
            return 2;
        client.Write(buf, amountRead);
    }

    return 0;
}
