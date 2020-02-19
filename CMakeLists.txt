cmake_minimum_required(VERSION 3.2)
project(mozart++)

set(CMAKE_MODULE_PATH "${CMCMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")

include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)

enable_testing()

#### Check C++14
if (WIN32)
    set(CMAKE_CXX_STANDARD 14)
else ()
    check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
    if (COMPILER_SUPPORTS_CXX14)
        set(CMAKE_CXX_STANDARD 14)
    else ()
        message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
    endif ()
endif ()

#### Check C99
if (WIN32)
    set(CMAKE_C_STANDARD 99)
else ()
    check_c_compiler_flag("-std=c99" COMPILER_SUPPORTS_C99)
    if (COMPILER_SUPPORTS_C99)
        set(CMAKE_C_STANDARD 99)
    else ()
        message(FATAL "The compiler ${CMAKE_C_COMPILER} has no C99 support. Please use a different C compiler.")
    endif ()
endif ()

message(STATUS "[Mozart++]: including Core")
add_subdirectory(mpp_core)
include_directories(mpp_core)

message(STATUS "[Mozart++]: including Foundation")
add_subdirectory(mpp_foundation)
include_directories(mpp_foundation)

message(STATUS "[Mozart++]: including String")
add_subdirectory(mpp_string)
include_directories(mpp_string)

message(STATUS "[Mozart++]: including System")
add_subdirectory(mpp_system)
include_directories(mpp_system)
