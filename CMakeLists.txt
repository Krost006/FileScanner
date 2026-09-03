cmake_minimum_required(VERSION 3.15)
project(FileScannerProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/lib)   # .lib / .a
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)   # .dll / .so
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)   # .exe

include(FetchContent)

FetchContent_Declare(
    hash_library
    GIT_REPOSITORY https://github.com/stbrumme/hash-library.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(hash_library)

set(HASHLIB_SRC_DIR ${hash_library_SOURCE_DIR})

add_library(HashLib
    ${HASHLIB_SRC_DIR}/md5.cpp
    ${HASHLIB_SRC_DIR}/md5.h
)

target_include_directories(HashLib PUBLIC ${HASHLIB_SRC_DIR})

add_subdirectory(FileScannerLib)

add_executable(scanner src/main.cpp)

target_link_libraries(scanner
    PRIVATE
        FileScannerLib
        HashLib
)

target_include_directories(scanner
    PRIVATE
        FileScannerLib
        ${HASHLIB_SRC_DIR}
)
