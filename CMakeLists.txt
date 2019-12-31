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
check_cxx_compiler_flag("/std:c++14" COMPILER_SUPPORTS_CXX14)
if (COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_STANDARD 14)
else ()
    message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif ()

#### Check C99
check_c_compiler_flag("-std=c99" COMPILER_SUPPORTS_C99)
check_c_compiler_flag("/std:c99" COMPILER_SUPPORTS_C99)
if (COMPILER_SUPPORTS_C99)
    set(CMAKE_C_STANDARD 99)
else ()
    message(FATAL "The compiler ${CMAKE_C_COMPILER} has no C99 support. Please use a different C compiler.")
endif ()

include_directories(.)

aux_source_directory(./mozart++ SRCS)
aux_source_directory(./src SRCS_IMPL)

## dummy target. DO NOT build this target directly or you will probably get an error.
add_library(mozart++ STATIC
        ${SRCS}
        ${SRCS_IMPL}
        )
set_target_properties(mozart++ PROPERTIES LINKER_LANGUAGE CXX)

## test targets here
add_executable(test-optional tests/test-optional.cpp)
add_executable(test-event tests/test-event.cpp)
add_executable(test-event-emit tests/test-event-emit.cpp)
add_executable(test-fp-curry tests/test-fp-curry.cpp)
add_executable(test-stream tests/test-stream.cpp)
add_executable(test-stringref tests/test-stringref.cpp)
add_executable(test-typelist tests/test-typelist.cpp)

target_link_libraries(test-event-emit mozart++)
