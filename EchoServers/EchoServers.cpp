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

#include "EchoServers.h"

#include <strapper/net/TcpListener.h>
#include <strapper/net/TcpSerializer.h>
#include <strapper/net/UdpSocket.h>
#include <strapper/net/IpAddress.h>

#include <string>
#include <iostream>

namespace strapper { namespace net {

void TcpEchoServer(uint16_t port)
{
    TcpListener listener(port);
    TcpSerializer client(listener.Accept());
    listener.Close();

    std::string message;

    while (message != "exit")
    {
        if (!client.Read(&message))
        {
            std::cout << "> Client gracefully closed the connection." << std::endl;
            return;
        }
        std::cout << "> " << message << std::endl;
        client.Write(message);
    }
    std::cout << "> Closing connection to client." << std::endl;
}


void UdpEchoServer(uint16_t port)
{
    UdpSocket client(port);

    unsigned constexpr MAX = 1000;
    char message[MAX + 1]{};

    while (strncmp(message, "exit", 5) != 0)
    {
        IpAddressV4 ip;
        uint16_t theirPort;
        unsigned const amountRead = client.Read(message, MAX, &ip, &theirPort);
        message[amountRead] = '\0';
        std::cout << "> " << message << std::endl;
        client.Write(message, amountRead, ip, theirPort);
    }
    std::cout << "> Closing socket." << std::endl;
}

} }
