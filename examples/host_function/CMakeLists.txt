add_executable(host_function_caller caller.c)
add_executable(host_function_receiver receiver.c)
target_link_libraries(host_function_caller PRIVATE robot_ipc)
target_link_libraries(host_function_receiver PRIVATE robot_ipc)
# target_link_libraries(shm_write PRIVATE robot_ipc)
set_target_properties(host_function_caller host_function_receiver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/host_function"
)

