cmake_minimum_required(VERSION 3.15)
project(test CXX)

# Bring in the runtime library
add_subdirectory(../../runtime runtime_build)

# test executable
add_executable(test main.cpp)

target_link_libraries(test PRIVATE leanclr)

target_compile_features(test PRIVATE cxx_std_17)

# Optional: place binaries under build tree for convenience
set_target_properties(test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
