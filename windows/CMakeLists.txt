cmake_minimum_required (VERSION 3.5)

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Network
add_library(Network STATIC
    SocketIncludes.h
    TcpListener.cpp
    TcpListener.h
    TcpSocket.cpp
    TcpSocket.h
    UdpSocket.cpp
    UdpSocket.h
    WinsockContextManager.h
)
target_link_libraries(Network 
    wsock32
    ws2_32
)
