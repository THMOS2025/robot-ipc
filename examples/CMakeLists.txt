set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples")

option(SUPER_VARIABLE_EXAMPLE "Enable building examples for host variable" ON)
option(SUPER_FUNCTION_EXAMPLE "Enable building examples for host function" ON)

if(SUPER_VARIABLE_EXAMPLE)
    add_subdirectory(host_variable)
    add_subdirectory(host_variable_struct)
    add_subdirectory(host_variable_integrity_check)
    add_subdirectory(host_variable_latency_test)
endif()

if(SUPER_FUNCTION_EXAMPLE)
    add_subdirectory(host_function)
    add_subdirectory(host_function_latency_test)
endif()
