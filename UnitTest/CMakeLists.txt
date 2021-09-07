# Unit Tests
add_executable(UnitTest
    Basic.cpp
    Endian.cpp
    Error.cpp
    TcpSerialize.cpp
    TestGlobals.h
    Timeout.h
)
target_link_libraries(UnitTest
    gtest_main
    Network
)
target_compile_options(UnitTest PRIVATE ${WARNING_FLAGS})
# set Visual Studio working directory
set_target_properties(UnitTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
