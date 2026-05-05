cmake_minimum_required(VERSION 3.15)
project(simple-aot CXX)

# Bring in the runtime library
add_subdirectory(../../runtime runtime_build)

# simple-aot executable

file(GLOB CPP_SOURCES "cpp/*.cpp")
add_executable(simple-aot main.cpp ${CPP_SOURCES})

target_link_libraries(simple-aot PRIVATE leanclr)


set_target_properties(simple-aot PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

# Optional: place binaries under build tree for convenience
set_target_properties(simple-aot PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(EMSCRIPTEN)
    set_target_properties(simple-aot PROPERTIES SUFFIX ".js")
    set(LEAN_SIMPLE_AOT_PINVOKE_JS "${CMAKE_CURRENT_SOURCE_DIR}/../../leanaot/Test/leanclr_test_pinvoke.js")
    set(LEAN_SIMPLE_AOT_WASM_LINK
        "SHELL:-s WASM=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=malloc,free"
        "SHELL:--js-library=${LEAN_SIMPLE_AOT_PINVOKE_JS}"
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Emscripten Debug: P/Invoke js-library ${LEAN_SIMPLE_AOT_PINVOKE_JS}")
        target_link_options(simple-aot PRIVATE ${LEAN_SIMPLE_AOT_WASM_LINK} -gsource-map)
    else()
        target_link_options(simple-aot PRIVATE ${LEAN_SIMPLE_AOT_WASM_LINK} -O2)
    endif()
endif()
