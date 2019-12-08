cmake_minimum_required(VERSION 3.14)
project(mozart++)

set(CMAKE_MODULE_PATH "${CMCMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")

include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)

enable_testing()

#### Check C++14
check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
if (COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else ()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif ()
set(CMAKE_CXX_STANDARD 14)

#### Check C99
check_c_compiler_flag("-std=c11" COMPILER_SUPPORTS_C99)
if (COMPILER_SUPPORTS_C99)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
else ()
    message(STATUS "The compiler ${CMAKE_C_COMPILER} has no C99 support. Please use a different C compiler.")
endif ()
set(CMAKE_C_STANDARD 99)

include_directories(mozart++)

## dummy target. DO NOT build this target directly or you will probably get an error.
add_library(mozart++ STATIC
        mozart++/utility.hpp
        )
set_target_properties(mozart++ PROPERTIES LINKER_LANGUAGE CXX)

## test targets here

