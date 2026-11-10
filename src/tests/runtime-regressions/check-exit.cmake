execute_process(COMMAND "${PROGRAM}" exit RESULT_VARIABLE code TIMEOUT 10)
if(NOT code STREQUAL "23")
    message(FATAL_ERROR "Environment.Exit must terminate with 23, observed ${code}")
endif()
