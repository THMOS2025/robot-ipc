add_executable(super_variable_latency_test_reader reader.c)
add_executable(super_variable_latency_test_writer writer.c)
target_link_libraries(super_variable_latency_test_reader PRIVATE robot_ipc)
target_link_libraries(super_variable_latency_test_writer PRIVATE robot_ipc)
set_target_properties(  super_variable_latency_test_reader
                        super_variable_latency_test_writer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/super_variable_latency_test"
)

