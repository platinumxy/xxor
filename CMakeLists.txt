cmake_minimum_required(VERSION 3.16)

project(xxor 
    VERSION 0.0.1
    DESCRIPTION "OTP-based messaging app"
    LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

include_directories(src)

set(SOURCES
    src/main.c
    src/key_management.c
    src/debugging.c
)

set(HEADERS
    src/main.h
    src/key_management.h
    src/debugging.h
    src/types.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testing)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(UNIX AND NOT APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _GNU_SOURCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")