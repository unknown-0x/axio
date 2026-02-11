cmake_minimum_required(VERSION 3.15)

include(cmake/Settings.cmake)
include(cmake/Utilities.cmake)

project(
    "project"
    VERSION 1.0.0
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options("/MP")
endif()

add_subdirectory(src)

if(BUILD_TESTING)
    include(CTest)
    include(FetchContent)
    enable_testing()
    
    FetchContent_Declare(
        simpletest
        GIT_REPOSITORY https://github.com/unknown-0x/simple-test.git
        GIT_TAG        main
    )

    FetchContent_MakeAvailable(simpletest)
    add_subdirectory(test)
endif()