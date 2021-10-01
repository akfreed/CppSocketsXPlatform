This solution contains a cross-platform API for sockets in C++14. It works on Windows and Posix systems.

There are five projects. Two of them are libraries, and three of them are executables. The sockets library is the part to check out. It can be found under `windows/` and `posix/`. CMake will include the appropriate source, depending on what platform you are building on. The other library contains the code for TCP and UDP echoservers.
