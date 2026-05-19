
cmake_minimum_required(VERSION 3.15)
project(leanclr CXX C)

option(BUILD_SHARED_LEANCLR "Build leanclr as a shared (dynamic) library" OFF)
option(LEANCLR_ENABLE_CPP_EXCEPTIONS "Enable C++ exceptions for runtime target" OFF)

set(LEANCLR_GC_ALGORITHM "MARK_SWEEP" CACHE STRING "GC algorithm: MARK_SWEEP or ZERO_GC")
option(LEANCLR_GC_DEBUG "Enable GC debug (quarantine, allocation sites)" ON)

set(RUNTIME_DIRS
    alloc
    codegen
    core
    icalls
    il2cpp
    interp
    intrinsics
    log
    metadata
    misc
    pinvokes
    platform
    public_impl
    utils
    vm
    vmutils
)

set(RUNTIME_SOURCES)
foreach(dir ${RUNTIME_DIRS})
    file(GLOB dir_cpp_sources "${CMAKE_CURRENT_LIST_DIR}/${dir}/*.cpp")
    list(APPEND RUNTIME_SOURCES ${dir_cpp_sources})
endforeach()

file(GLOB dir_c_sources "${CMAKE_CURRENT_LIST_DIR}/3rd/zlib/*.c")
# Emscripten: omit gz*.c (gzip file I/O via write/close). Runtime only uses in-memory deflate/inflate.
if(EMSCRIPTEN)
    list(FILTER dir_c_sources EXCLUDE REGEX "/gz[^/]*\\.c$")
endif()
list(APPEND RUNTIME_SOURCES ${dir_c_sources})

file(GLOB_RECURSE gc_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/gc/*.cpp")
list(APPEND RUNTIME_SOURCES ${gc_sources})

if(BUILD_SHARED_LEANCLR)
    add_library(leanclr SHARED ${RUNTIME_SOURCES})
else()
    add_library(leanclr STATIC ${RUNTIME_SOURCES})
endif()

target_compile_definitions(leanclr PRIVATE LEANCLR_DLL_EXPORTS LEANCLR_USE_SHIPPING_ZLIB=1)
target_compile_definitions(leanclr PUBLIC UNITY_VERSION=60000400)

if(LEANCLR_GC_ALGORITHM STREQUAL "MARK_SWEEP")
    target_compile_definitions(leanclr PRIVATE LEANCLR_GC_MARK_SWEEP=1)
elseif(LEANCLR_GC_ALGORITHM STREQUAL "ZERO_GC")
    target_compile_definitions(leanclr PRIVATE LEANCLR_GC_ZERO_GC=1)
else()
    message(FATAL_ERROR "Unknown LEANCLR_GC_ALGORITHM: ${LEANCLR_GC_ALGORITHM} (use MARK_SWEEP or ZERO_GC)")
endif()
if(LEANCLR_GC_DEBUG)
    target_compile_definitions(leanclr PUBLIC LEANCLR_GC_DEBUG=1)
endif()

target_include_directories(leanclr PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

set_target_properties(leanclr PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    if(LEANCLR_ENABLE_CPP_EXCEPTIONS)
        target_compile_options(leanclr PRIVATE /EHsc /MP)
    else()
        target_compile_options(leanclr PRIVATE /EHs-c- /MP)
    endif()
elseif(EMSCRIPTEN)

    target_compile_options(leanclr PRIVATE
        $<$<BOOL:${LEANCLR_ENABLE_CPP_EXCEPTIONS}>:-fexceptions>
        $<$<NOT:$<BOOL:${LEANCLR_ENABLE_CPP_EXCEPTIONS}>>:-fno-exceptions>
        -fno-rtti
        -O2
    )

    target_link_options(leanclr PRIVATE
        "SHELL: -s WASM=1"
        "SHELL: -s ALLOW_MEMORY_GROWTH=1"
        -O2
    )

else()
    if(LEANCLR_ENABLE_CPP_EXCEPTIONS)
        target_compile_options(leanclr PRIVATE -fexceptions)
    else()
        target_compile_options(leanclr PRIVATE -fno-exceptions)
    endif()
endif()

# Implicit narrowing conversions (truncation) may lose data.
if(MSVC)
    target_compile_options(leanclr PRIVATE
        /w14244   # C4244: conversion; possible loss of data
        /w14267   # C4267: conversion from size_t to smaller type
    )
else()
    target_compile_options(leanclr PRIVATE -Wconversion)
endif()

# LEANCLR_PLATFORM_* macros are derived from compiler-predefined macros in
# build_config.h so both CMake and non-CMake consumers see the same view.
