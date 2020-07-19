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

// Contains prototypes for the TCP and UDP echo server functions.

#pragma once

// return 0 for success
// return 1 for listener error
// return 2 for accept error
// return 3 for unexpected close
int TcpEchoServer(const char* port);


// return 0 for success
// return 1 for socket error
// return 2 for unexpected close
int UdpEchoServer(unsigned short port);
