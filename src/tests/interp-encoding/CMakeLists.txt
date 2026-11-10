cmake_minimum_required(VERSION 3.15)
project(interp_encoding_tests CXX)

# Exercise the encoder without runtime initialization or managed assemblies.
add_executable(interp-encoding-tests
    main.cpp
    ../../runtime/interp/ll_opcodes.cpp
)
target_include_directories(interp-encoding-tests PRIVATE ../../runtime)
set_target_properties(interp-encoding-tests PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
enable_testing()
add_test(NAME interp-encoding COMMAND interp-encoding-tests)
