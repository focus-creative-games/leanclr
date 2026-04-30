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
