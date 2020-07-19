//============================================================================
// Copyright (c) 2018 Alexander Freed
// 
// Contains prototypes for the TCP and UDP echo server functions.
//============================================================================
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
