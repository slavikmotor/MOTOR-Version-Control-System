cmake_minimum_required(VERSION 3.14)
project(Motor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

option(USE_SYSTEM_ZLIB "Use system ZLIB if available" ON)
if(USE_SYSTEM_ZLIB)
    find_package(ZLIB QUIET)
endif()

set(SOURCES
    src/main.cpp
    src/objects/object.cpp
    src/objects/blob.cpp
    src/objects/tree.cpp
    src/objects/commit.cpp
    src/objects/tag.cpp
    src/repository.cpp
    src/reference.cpp
    src/index.cpp
    src/utils.cpp
)

if(NOT ZLIB_FOUND)
    list(APPEND SOURCES src/compression.cpp)
    add_definitions(-DUSE_INTERNAL_COMPRESSION)
endif()

add_executable(motor ${SOURCES})

target_include_directories(motor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(motor PRIVATE 
    OpenSSL::Crypto 
)

if(ZLIB_FOUND)
    target_include_directories(motor PRIVATE ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(motor PRIVATE ${ZLIB_LIBRARIES})
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0.0")
        target_link_libraries(motor PRIVATE stdc++fs)
    endif()
endif()

install(TARGETS motor DESTINATION bin) 