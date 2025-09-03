add_executable(caller caller.c)
add_executable(receiver receiver.c)
target_link_libraries(caller PRIVATE robot_ipc)
target_link_libraries(receiver PRIVATE robot_ipc)
# target_link_libraries(shm_write PRIVATE robot_ipc)
set_target_properties(caller receiver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/host_function_latency_test"
)

