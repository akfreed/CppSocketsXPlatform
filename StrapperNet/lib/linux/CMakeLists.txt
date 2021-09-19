file(GLOB_RECURSE SRCS *.h *.cpp)

target_sources(StrapperNet PRIVATE
    ${SRCS}
)
