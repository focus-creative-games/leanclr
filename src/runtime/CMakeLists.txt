
cmake_minimum_required(VERSION 3.15)
project(leanclr CXX)

# Ninja / single-config generators: produces compile_commands.json for IDE IntelliSense (VS generator omits it on Windows).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json for clangd / cpptools" FORCE)

option(BUILD_SHARED_LEANCLR "Build leanclr as a shared (dynamic) library" OFF)
option(LEANCLR_ENABLE_CPP_EXCEPTIONS "Enable C++ exceptions for runtime target" OFF)

set(RUNTIME_DIRS
    alloc
    codegen
    core
    gc
    icalls
    il2cpp
    interp
    intrinsics
    log
    metadata
    misc
    platform
    public_impl
    utils
    vm
    vmutils
)

set(RUNTIME_SOURCES)
foreach(dir ${RUNTIME_DIRS})
    file(GLOB dir_sources "${CMAKE_CURRENT_LIST_DIR}/${dir}/*.cpp")
    list(APPEND RUNTIME_SOURCES ${dir_sources})
endforeach()

if(BUILD_SHARED_LEANCLR)
    add_library(leanclr SHARED ${RUNTIME_SOURCES})
else()
    add_library(leanclr STATIC ${RUNTIME_SOURCES})
endif()

target_compile_definitions(leanclr PRIVATE LEANCLR_DLL_EXPORTS)
target_compile_definitions(leanclr PUBLIC UNITY_VERSION=60000400)

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
