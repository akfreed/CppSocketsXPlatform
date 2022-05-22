cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE INCL include/*.h)
file(GLOB SRCS LIST_DIRECTORIES false lib/*.h lib/*.cpp)

# StrapperNet
add_library(StrapperNet STATIC
    ${INCL}
    ${SRCS}
)
target_include_directories(StrapperNet PUBLIC include/)
target_link_libraries(StrapperNet PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
)
target_compile_options(StrapperNet PRIVATE ${WARNING_FLAGS})

# Add platform-dependent code.
if(WIN32)
    add_subdirectory(lib/windows/)
elseif(LINUX OR UNIX)
    add_subdirectory(lib/linux/)
endif()

if(${BUILD_WITH_CLANG_TIDY})
    # Optionally enable clang-tidy on build for MSVC builds
    set_target_properties(StrapperNet PROPERTIES
        VS_GLOBAL_RunCodeAnalysis true
        VS_GLOBAL_EnableClangTidyCodeAnalysis true
    )
endif()
