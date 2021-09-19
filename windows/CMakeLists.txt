cmake_minimum_required (VERSION 3.15)

# C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Network
add_library(Network STATIC
    ErrorCode.cpp
    ErrorCode.h
    IpAddress.cpp
    IpAddress.h
    SocketError.h
    SocketHandle.cpp
    SocketHandle.h
    SocketIncludes.h
    TcpListener.cpp
    TcpListener.h
    TcpListenerBase.cpp
    TcpListenerBase.h
    TcpListenerEc.cpp
    TcpListenerEc.h
    TcpSerializer.cpp
    TcpSerializer.h
    TcpSocket.cpp
    TcpSocket.h
    TcpSocketBase.cpp
    TcpSocketBase.h
    TcpSocketEc.cpp
    TcpSocketEc.h
    UdpSocket.cpp
    UdpSocket.h
    UdpSocketBase.cpp
    UdpSocketBase.h
    UdpSocketEc.cpp
    UdpSocketEc.h
    WinsockContext.cpp
    WinsockContext.h
)
target_include_directories(Network PUBLIC .)
target_link_libraries(Network 
    wsock32
    ws2_32
    ${CMAKE_THREAD_LIBS_INIT}
)
target_compile_options(Network PRIVATE ${WARNING_FLAGS})
