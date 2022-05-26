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

if(${BUILD_WITH_CLANG_TIDY})
    # Optionally enable clang-tidy on build for MSVC builds
    set_target_properties(EchoServers PROPERTIES
        VS_GLOBAL_RunCodeAnalysis true
        VS_GLOBAL_EnableClangTidyCodeAnalysis true
    )
endif()


# Tcp Echo Server
add_executable(TcpEchoServer
    TcpEchoServerMain.cpp
)
target_link_libraries(TcpEchoServer
    EchoServers
)
target_compile_options(TcpEchoServer PRIVATE ${WARNING_FLAGS})
# set Visual Studio working directory
set_target_properties(TcpEchoServer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
if(${BUILD_WITH_CLANG_TIDY})
    # Optionally enable clang-tidy on build for MSVC builds
    set_target_properties(TcpEchoServer PROPERTIES
        VS_GLOBAL_RunCodeAnalysis true
        VS_GLOBAL_EnableClangTidyCodeAnalysis true
    )
endif()


# Udp Echo Server
add_executable(UdpEchoServer
    UdpEchoServerMain.cpp
)
target_link_libraries(UdpEchoServer
    EchoServers
)
target_compile_options(UdpEchoServer PRIVATE ${WARNING_FLAGS})
# set Visual Studio working directory
set_target_properties(UdpEchoServer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
if(${BUILD_WITH_CLANG_TIDY})
    # Optionally enable clang-tidy on build for MSVC builds
    set_target_properties(UdpEchoServer PROPERTIES
        VS_GLOBAL_RunCodeAnalysis true
        VS_GLOBAL_EnableClangTidyCodeAnalysis true
    )
endif()

# Tcp Echo Server
add_executable(TcpEchoClient
    TcpEchoClientMain.cpp
)
target_link_libraries(TcpEchoClient
    StrapperNet
)
target_compile_options(TcpEchoClient PRIVATE ${WARNING_FLAGS})
# set Visual Studio working directory
set_target_properties(TcpEchoClient PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
if(${BUILD_WITH_CLANG_TIDY})
    # Optionally enable clang-tidy on build for MSVC builds
    set_target_properties(TcpEchoClient PROPERTIES
        VS_GLOBAL_RunCodeAnalysis true
        VS_GLOBAL_EnableClangTidyCodeAnalysis true
    )
endif()

# Udp Echo Client
add_executable(UdpEchoClient
    UdpEchoClientMain.cpp
)
target_link_libraries(UdpEchoClient
    StrapperNet
)
target_compile_options(UdpEchoClient PRIVATE ${WARNING_FLAGS})
# set Visual Studio working directory
set_target_properties(UdpEchoClient PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
if(${BUILD_WITH_CLANG_TIDY})
    # Optionally enable clang-tidy on build for MSVC builds
    set_target_properties(UdpEchoClient PROPERTIES
        VS_GLOBAL_RunCodeAnalysis true
        VS_GLOBAL_EnableClangTidyCodeAnalysis true
    )
endif()
