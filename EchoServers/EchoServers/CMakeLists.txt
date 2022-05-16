# Echo Servers (library)
add_library(EchoServers STATIC
    EchoServers.cpp
    EchoServers.h
)
target_include_directories(EchoServers PUBLIC .)
target_link_libraries(EchoServers
    StrapperNet
)
target_compile_options(EchoServers PRIVATE ${WARNING_FLAGS})


# Tcp Echo Server
add_executable(TcpEcho
    TcpEchoMain.cpp
)
target_link_libraries(TcpEcho
    EchoServers
)
target_compile_options(TcpEcho PRIVATE ${WARNING_FLAGS})
# set Visual Studio working directory
set_target_properties(TcpEcho PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")


# Udp Echo Server
add_executable(UdpEcho
    UdpEchoMain.cpp
)
target_link_libraries(UdpEcho
    EchoServers
)
target_compile_options(UdpEcho PRIVATE ${WARNING_FLAGS})
# set Visual Studio working directory
set_target_properties(UdpEcho PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")