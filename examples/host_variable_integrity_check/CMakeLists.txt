add_executable(host_variable_integrity_test_reader reader.c)
add_executable(host_variable_integrity_test_writer writer.c)
target_link_libraries(host_variable_integrity_test_reader PRIVATE robot_ipc)
target_link_libraries(host_variable_integrity_test_writer PRIVATE robot_ipc)
set_target_properties(  host_variable_integrity_test_reader 
                        host_variable_integrity_test_writer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY 
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/host_variable_integrity_check"
)

