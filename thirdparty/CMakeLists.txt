# Configure Google Test
set(BUILD_GMOCK OFF CACHE BOOL "Build GMock")
set(INSTALL_GTEST OFF CACHE BOOL "Install GTest")
set(gtest_force_shared_crt ON CACHE BOOL "Force Shared CRT")
add_subdirectory(googletest/)
