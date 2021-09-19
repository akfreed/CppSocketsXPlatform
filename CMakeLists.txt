cmake_minimum_required (VERSION 3.15)

# C++11
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

# Solution
project (CppSocketsXPlat)

# Warning flags
if(MSVC)
    set(WARNING_FLAGS /W4 /WX
        /experimental:external # VS 16.9 and older
        /external:I ${CMAKE_SOURCE_DIR}/thirdparty/
        /external:W0 # Causes an unfortunate warning due to a missing flag for the new 16.10 external headers flag. Fixed in CMake 3.21.
    )
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(WARNING_FLAGS -Wall -Wextra -pedantic -Werror -Wconversion)
endif()


# Dependencies
find_package(Threads REQUIRED)
add_subdirectory(thirdparty/)

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
