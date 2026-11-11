cmake_minimum_required(VERSION 3.15)
project(custom-pinvoke CXX)

# Bring in the runtime library
add_subdirectory(../../runtime runtime_build)

# custom-pinvoke executable
add_executable(custom-pinvoke main.cpp)

target_link_libraries(custom-pinvoke PRIVATE leanclr)

target_compile_features(custom-pinvoke PRIVATE cxx_std_17)

# Optional: place binaries under build tree for convenience
set_target_properties(custom-pinvoke PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
