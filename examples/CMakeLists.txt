set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples")

option(SUPER_VARIABLE_EXAMPLE "Enable building examples for super variable" ON)
option(SUPER_FUNCTION_EXAMPLE "Enable building examples for super function" ON)

if(SUPER_VARIABLE_EXAMPLE)
    add_subdirectory(super_variable)
    add_subdirectory(super_variable_integrity_check)
    add_subdirectory(super_variable_latency_test)
endif()

if(SUPER_FUNCTION_EXAMPLE)
    add_subdirectory(super_function)
    add_subdirectory(super_function_latency_test)
endif()
