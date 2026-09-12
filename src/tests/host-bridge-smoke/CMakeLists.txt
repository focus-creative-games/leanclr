cmake_minimum_required(VERSION 3.15)
project(host_bridge_smoke CXX)

add_executable(host-bridge-smoke main.cpp)

target_include_directories(host-bridge-smoke PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../runtime
)

set_target_properties(host-bridge-smoke PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
