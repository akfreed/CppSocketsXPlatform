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

#include <strapper/net/TcpSerializer.h>

#include <exception>
#include <iostream>
#include <string>

using namespace strapper::net;

namespace {

void TcpEchoClient(std::string const& ip, uint16_t port)
{
    TcpSerializer server(TcpSocket(ip, port));

    std::string message;

    while (message != "exit")
    {
        std::cout << "< " << std::flush;
        std::getline(std::cin, message);

        if (!std::cin.good())
        {
            if (std::cin.eof())
                std::cout << "End of input." << std::endl;
            else
                std::cout << "Some issue closed stdin." << std::endl;
            break;
        }

        server.Write(message);

        std::string response;
        bool const stillConnected = server.Read(&response);
        if (!stillConnected)
        {
            std::cout << "Server disconnected." << std::endl;
            break;
        }

        std::cout << "> " << response << std::endl;
    }

    std::cout << "Quitting." << std::endl;
}

}  // namespace

int main()
{
    try
    {
        TcpEchoClient("127.0.0.1", 11111);
        return EXIT_SUCCESS;
    }
    catch (std::exception const& e)
    {
        std::cout << "Exception occured.\n"
                  << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown exception occured." << std::endl;
    }
    return EXIT_FAILURE;
}
