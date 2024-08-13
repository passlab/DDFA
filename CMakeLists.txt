cmake_minimum_required(VERSION 3.2)
project(DDFA)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_BUILD_TYPE Debug)

set(DDFA_VERSION_MAJOR 0)
set(DDFA_VERSION_MINOR 1)
set(DDFA_VERSION ${DDFA_VERSION_MAJOR}.${DDFA_VERSION_MINOR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/ddfa_config.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/src/ddfa_config.h" @ONLY)

set(SOURCE_FILES
    src/ddfa.h
    src/ddfa.c
    src/datamap.h
    src/datamap.c
    src/callgraph.h
    src/callgraph.c
    src/basicdef.h
    src/ddfa_config.h
)

add_library(ddfa SHARED ${SOURCE_FILES})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ddfa m)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/src/ddfa_config.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ddfa.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/basicdef.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/datamap.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/callgraph.h
        DESTINATION include)

install(TARGETS ddfa
        LIBRARY DESTINATION lib
        )
