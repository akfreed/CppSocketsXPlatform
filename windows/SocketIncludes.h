//============================================================================
// Copyright (c) 2017 Alexander Freed
// 
// This file should be included before windows.h
//============================================================================
#pragma once

//----------------------------------------------------------------------------------------------//
// inet_pton (used in UdpSocket.cpp) requires that we only compile for Vista and later.         //
// These defines should be placed before including Windows.h or any other header but w32api.h.  //
// https://stackoverflow.com/questions/4890753/inetntop-cant-find-which-header-it-is-using      //
// Otherwise, use Windows XP as the version.                                                    //
//#include <SdkDdkver.h>                                                                        //
#define NTDDI_VERSION NTDDI_VISTA                                                               //
#define WINVER _WIN32_WINNT_VISTA                                                               //
#define _WIN32_WINNT _WIN32_WINNT_VISTA   //----------------------------------------------------//

// prevent windows.h from including winsock.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
//#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")
