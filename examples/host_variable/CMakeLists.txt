if(BUILD_C)
    add_executable(reader_c reader.cpp)
    add_executable(writer_c writer.cpp)
    target_link_libraries(reader_c PRIVATE robot_ipc)
    target_link_libraries(writer_c PRIVATE robot_ipc)
    set_target_properties(reader_c writer_c PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/host_variable"
    )
endif()

if(BUILD_CPP)
    add_executable(reader_cpp reader.cpp)
    add_executable(writer_cpp writer.cpp)
    target_link_libraries(reader_cpp PRIVATE robot_ipc)
    target_link_libraries(writer_cpp PRIVATE robot_ipc)
    set_target_properties(reader_cpp writer_cpp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/host_variable"
    )
endif()
