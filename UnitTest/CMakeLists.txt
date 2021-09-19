# Unit Tests
add_executable(UnitTest
    UnitTestBasic.cpp
    UnitTestEndian.cpp
    UnitTestError.cpp
    UnitTestSerialize.cpp
    TestGlobals.h
    Timeout.h
    UnitTestIpAddress.cpp
    UnitTestMain.cpp
)
target_link_libraries(UnitTest
    gtest
    StrapperNet
)
target_compile_options(UnitTest PRIVATE ${WARNING_FLAGS})
# set Visual Studio working directory
set_target_properties(UnitTest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
