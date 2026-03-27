cmake_minimum_required(VERSION 3.15)
project(test CXX)

# Bring in the runtime library
add_subdirectory(../../runtime runtime_build)

# test executable
add_executable(test main.cpp)

target_link_libraries(test PRIVATE leanclr)

set_target_properties(test PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

# Optional: place binaries under build tree for convenience
set_target_properties(test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
