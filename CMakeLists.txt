cmake_minimum_required (VERSION 3.5)

# C++11
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

# Solution
project (CppSocketsXPlat)

# Include directories
include_directories(
    .
    EchoServers/
)

# Use different source files for sockets depending on platform
if (WIN32)
    add_subdirectory(windows)
    include_directories(windows)
elseif (LINUX OR UNIX OR APPLE)
    add_subdirectory(posix)
    include_directories(posix)
endif (WIN32)


# Warning flags
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
  if(NOT CMAKE_CXX_FLAGS MATCHES "/WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
endif()


# Projects

# Echo Servers (library)
add_library(EchoServers STATIC
    EchoServers/EchoServers.cpp
    EchoServers/EchoServers.h
)
target_link_libraries(EchoServers
    Network
)


# Tcp Echo Server
add_executable(TcpEcho
    TcpEchoMain.cpp
)
target_link_libraries(TcpEcho
    Network
    EchoServers
)
# set Visual Studio working directory
set_target_properties(TcpEcho PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")


# Udp Echo Server
add_executable(UdpEcho
    UdpEchoMain.cpp
)
target_link_libraries(UdpEcho
    Network
    EchoServers
)
# set Visual Studio working directory
set_target_properties(UdpEcho PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")


# Unit Tests
add_executable(UnitTest
    UnitTest/Basic.cpp
    UnitTest/Endian.cpp
    UnitTest/Error.cpp
    UnitTest/TestReport.h
    UnitTest/UnitTestMain.cpp
    UnitTest/UnitTestMain.h
)
target_link_libraries(UnitTest
    Network
)
# set Visual Studio working directory
set_target_properties(UnitTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")


# Set Unit Tests as the start-up project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT UnitTest)
