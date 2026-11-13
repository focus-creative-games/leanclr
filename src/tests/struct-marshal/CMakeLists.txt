cmake_minimum_required(VERSION 3.15)
project(struct_marshal_contracts C CXX)
add_subdirectory(../../tools/leanrun leanrun_build)
add_executable(struct-marshal-memory native.cpp)
target_link_libraries(struct-marshal-memory PRIVATE leanclr)
set(LEANCLR_MARSHAL_MANAGED_DIR "" CACHE PATH "Directory containing StructMarshalTests.dll")
set(LEANCLR_MARSHAL_RUNTIME_DIR "" CACHE PATH "Directory containing the selected BCL")
if(EMSCRIPTEN)
    foreach(target leanrun struct-marshal-memory)
        set_target_properties(${target} PROPERTIES SUFFIX ".js")
        target_link_options(${target} PRIVATE -sENVIRONMENT=node -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=1 -lnodefs.js
            "--pre-js=${CMAKE_CURRENT_SOURCE_DIR}/../../benchmarks/aot-baseline/nodefs.js")
    endforeach()
endif()
enable_testing()
foreach(entry All PrimitiveMatrix ConstructorSemantics NullPointer FailureCleanup Pressure)
    add_test(NAME marshal-${entry} COMMAND leanrun -l "${LEANCLR_MARSHAL_MANAGED_DIR}" -l "${LEANCLR_MARSHAL_RUNTIME_DIR}"
        -e "StructMarshalTests.Program::${entry}" StructMarshalTests)
endforeach()
add_test(NAME marshal-native-memory COMMAND struct-marshal-memory "${LEANCLR_MARSHAL_MANAGED_DIR}" "${LEANCLR_MARSHAL_RUNTIME_DIR}")

set_tests_properties(marshal-All marshal-PrimitiveMatrix marshal-ConstructorSemantics marshal-NullPointer marshal-FailureCleanup marshal-Pressure marshal-native-memory
    PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
