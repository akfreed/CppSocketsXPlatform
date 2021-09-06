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

// This is the only file in the project and it simply runs a UDP echo server. 

#include "WinsockContextManager.h"
#include <iostream>
#include "EchoServers.h"


int main()
{
    WinsockContextManager wcm;
    if (!wcm.IsInitialized())
    {
        std::cout << "Unable to initialize winsock.\n";
        return 4;
    }

    int result = UdpEchoServer(11111);

    switch (result)
    {
    case 0:
        std::cout << "Graceful close.\n";
        return 0;
        break;

    case 1:
        std::cout << "ERROR: Socket Error.\n";
        return 1;
        break;

    case 2:
        std::cout << "Other end terminated connection.\n";
        return 3;
        break;

    default:
        std::cout << "ERROR: Unknown return code.\n";
        return 5;
        break;
    }
}
