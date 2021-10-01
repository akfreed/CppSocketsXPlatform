file(GLOB_RECURSE SRCS *.h *.cpp)

target_sources(StrapperNet PRIVATE
    ${SRCS}
)
target_link_libraries(StrapperNet PUBLIC
    wsock32
    ws2_32
)
