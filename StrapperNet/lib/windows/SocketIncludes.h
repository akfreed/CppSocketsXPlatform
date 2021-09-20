// ==================================================================
// Copyright 2017, 2021 Alexander K. Freed
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

// This file should be included before windows.h

#pragma once

#ifdef _INC_WINDOWS
    static_assert(false, "This header must be included before windows.h");
#endif

//----------------------------------------------------------------------------------------------//
// inet_pton (used in UdpSocket.cpp) requires that we only compile for Vista and later.         //
// These defines should be placed before including Windows.h or any other header but w32api.h.  //
// https://stackoverflow.com/questions/4890753/inetntop-cant-find-which-header-it-is-using      //
// Otherwise, use Windows XP as the version.                                                    //
#define NTDDI_VERSION NTDDI_VISTA                                                               //
#define WINVER _WIN32_WINNT_VISTA                                                               //
#define _WIN32_WINNT _WIN32_WINNT_VISTA   //----------------------------------------------------//

// prevent windows.h from including winsock.h
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN 
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#ifndef INCL_EXTRA_HTON_FUNCTIONS
    #define INCL_EXTRA_HTON_FUNCTIONS
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
//#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")
