
cmake_minimum_required(VERSION 3.15)
project(leanclr CXX)

option(BUILD_SHARED_LEANCLR "Build leanclr as a shared (dynamic) library" OFF)

set(RUNTIME_DIRS
    alloc
    codegen
    core
    gc
    icalls
    interp
    intrinsics
    log
    metadata
    misc
    platform
    utils
    vm
    public_impl
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

set(_include_dirs ${CMAKE_CURRENT_LIST_DIR})
foreach(dir ${RUNTIME_DIRS})
    list(APPEND _include_dirs "${CMAKE_CURRENT_LIST_DIR}/${dir}")
endforeach()

target_include_directories(leanclr PUBLIC ${_include_dirs})
target_include_directories(leanclr PUBLIC ${CMAKE_CURRENT_LIST_DIR}/3rd)

set_target_properties(leanclr PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    target_compile_options(leanclr PRIVATE /EHs-c- /MP)
elseif(EMSCRIPTEN)

    target_compile_options(leanclr PRIVATE
        -fno-exceptions
        -fno-rtti
        -O2
    )

    target_link_options(leanclr PRIVATE
        "SHELL: -s WASM=1"
        "SHELL: -s ALLOW_MEMORY_GROWTH=1"
        -O2
    )

else()
    target_compile_options(leanclr PRIVATE -fno-exceptions)
endif()

if(WIN32)
    target_compile_definitions(leanclr PUBLIC LEANCLR_PLATFORM_WIN)
elseif(APPLE)
    if(IOS)
        target_compile_definitions(leanclr PUBLIC LEANCLR_PLATFORM_IOS LEANCLR_PLATFORM_POSIX)
    else()
        target_compile_definitions(leanclr PUBLIC LEANCLR_PLATFORM_MAC LEANCLR_PLATFORM_POSIX)
    endif()
elseif(ANDROID)
    target_compile_definitions(leanclr PUBLIC LEANCLR_PLATFORM_ANDROID LEANCLR_PLATFORM_POSIX)
elseif(EMSCRIPTEN)
    target_compile_definitions(leanclr PUBLIC LEANCLR_PLATFORM_WASM)
elseif(UNIX)
    target_compile_definitions(leanclr PUBLIC LEANCLR_PLATFORM_LINUX LEANCLR_PLATFORM_POSIX)
else()
    target_compile_definitions(leanclr PUBLIC LEANCLR_PLATFORM_UNKNOWN)
endif()
