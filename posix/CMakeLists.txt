cmake_minimum_required (VERSION 3.15)

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Network
add_library(Network STATIC
    TcpListener.cpp
    TcpListener.h
    TcpSocket.cpp
    TcpSocket.h
    UdpSocket.cpp
    UdpSocket.h
    WinsockContextManager.h
)
target_include_directories(Network PUBLIC .)
target_link_libraries(Network
    ${CMAKE_THREAD_LIBS_INIT}
)
target_compile_options(Network PRIVATE ${WARNING_FLAGS})
