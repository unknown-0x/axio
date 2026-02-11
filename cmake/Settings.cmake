option(BUILD_TESTING "Build testing" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
find_program(CLANG_FORMAT_EXE NAMES clang-format)

if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy: ${CLANG_TIDY_EXE}")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
else()
    message(WARNING "clang-tidy not found, set ENABLE_CLANG_TIDY to OFF")
    set(ENABLE_CLANG_TIDY OFF CACHE BOOL "Disable clang-tidy because executable not found" FORCE)
endif()

if(CLANG_FORMAT_EXE)
    message(STATUS "clang-format: ${CLANG_FORMAT_EXE}")
else()
    message(WARNING "clang-format not found")
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")
endif()

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "PROJECT_ROOT: ${PROJECT_ROOT}")
message(STATUS "ENABLE_CLANG_TIDY: ${ENABLE_CLANG_TIDY}")
message(STATUS "WARNINGS_AS_ERRORS: ${WARNINGS_AS_ERRORS}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message(STATUS "BUILD_TESTING: ${BUILD_TESTING}")