cmake_minimum_required(VERSION 3.15)
project(lean CXX)

# Bring in the runtime library
add_subdirectory(../../runtime runtime_build)

# Startup executable
add_executable(lean main.cpp)

target_link_libraries(lean PRIVATE leanclr)

set_target_properties(lean PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

# Optional: place binaries under build tree for convenience
set_target_properties(lean PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# WebAssembly specific configuration
if(EMSCRIPTEN)
    # Set output as .js + .wasm (no HTML wrapper)
    set_target_properties(lean PROPERTIES SUFFIX ".js")
    set(LEAN_WASM_COMMON_LINK_OPTIONS
        "SHELL:-s WASM=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s EXPORTED_FUNCTIONS=['_load_assembly','_invoke_method','_initialize_runtime','_allocate_bytes','_malloc','_free']"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','getValue','setValue','intArrayFromString','HEAPU8','allocate']"
        "SHELL:-s MODULARIZE=1"
        "SHELL:-s EXPORT_NAME='createStartupModule'"
        "SHELL:-s ENVIRONMENT=web,worker"
        "SHELL:--js-library=${CMAKE_CURRENT_SOURCE_DIR}/mylib.js"
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Emscripten Debug build: enabling debug symbols and source map.")
        target_link_options(lean PRIVATE ${LEAN_WASM_COMMON_LINK_OPTIONS} -gsource-map)
    else()
        target_link_options(lean PRIVATE ${LEAN_WASM_COMMON_LINK_OPTIONS} -O2)
    endif()
endif()
