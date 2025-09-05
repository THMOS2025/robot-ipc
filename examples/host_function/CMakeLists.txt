if(BUILD_C)
    add_executable(host_function_caller_c caller.c)
    add_executable(host_function_receiver_c receiver.c)
    target_link_libraries(host_function_caller_c PRIVATE robot_ipc)
    target_link_libraries(host_function_receiver_c PRIVATE robot_ipc)
    set_target_properties(
        host_function_caller_c      host_function_receiver_c
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/host_function")
endif()

if(BUILD_CPP)
    add_executable(host_function_caller_cpp caller.cpp)
    add_executable(host_function_receiver_cpp receiver.cpp)
    target_link_libraries(host_function_caller_cpp PRIVATE robot_ipc)
    target_link_libraries(host_function_receiver_cpp PRIVATE robot_ipc)
    set_target_properties(
        host_function_caller_cpp    host_function_receiver_cpp
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/host_function"
    )
endif()

