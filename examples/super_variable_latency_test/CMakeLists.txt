add_executable(shm_read shm_reader.c)
add_executable(shm_write shm_writer.c)
target_link_libraries(shm_read PRIVATE robot_ipc)
target_link_libraries(shm_write PRIVATE robot_ipc)
set_target_properties(shm_read shm_write PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/super_variable_latency_test"
)

