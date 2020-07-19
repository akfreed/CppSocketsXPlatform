//============================================================================
// Copyright (c) 2018 Alexander Freed
// 
// This is the only file in the project and it simply runs a UDP echo server. 
//============================================================================

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
