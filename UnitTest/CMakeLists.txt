file(GLOB_RECURSE SRCS *.h *.cpp)

# Unit Tests
add_executable(UnitTest
    ${SRCS}
)
target_link_libraries(UnitTest
    gtest
    StrapperNet
)
target_compile_options(UnitTest PRIVATE ${WARNING_FLAGS})
# Set Visual Studio working directory
set_target_properties(UnitTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
# Optionally enable clang-tidy on build for MSVC builds
if(${BUILD_WITH_CLANG_TIDY})
    set_target_properties(UnitTest PROPERTIES
        VS_GLOBAL_RunCodeAnalysis true
        VS_GLOBAL_EnableClangTidyCodeAnalysis true
    )
endif()
