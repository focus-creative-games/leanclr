cmake_minimum_required(VERSION 3.15)
project(leanrun CXX)

# Bring in the runtime library
add_subdirectory(../../runtime runtime_build)

# leanrun executable
add_executable(leanrun main.cpp)

target_link_libraries(leanrun PRIVATE leanclr)

target_compile_features(leanrun PRIVATE cxx_std_17)

# Optional: place binaries under build tree for convenience
set_target_properties(leanrun PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
