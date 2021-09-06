cmake_minimum_required (VERSION 3.10)

# C++14
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

# Solution
project (CppSocketsXPlat)

# Warning flags
if(MSVC)
    set(WARNING_FLAGS /W4 /WX)
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(WARNING_FLAGS -Wall -Wextra -pedantic -Werror)
endif()


# Dependencies
find_package(Threads REQUIRED)

# Projects
# Use different source files for sockets depending on platform
if(WIN32)
    add_subdirectory(windows)
    include_directories(windows)
elseif(LINUX OR UNIX OR APPLE)
    add_subdirectory(posix)
    include_directories(posix)
endif()
add_subdirectory(EchoServers)
add_subdirectory(UnitTest)

# Set Unit Tests as the start-up project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT UnitTest)
