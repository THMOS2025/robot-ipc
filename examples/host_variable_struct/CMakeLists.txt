if(BUILD_C)
    add_executable(reader_struct reader.c)
    add_executable(writer_struct writer.c)
    target_link_libraries(reader_struct PRIVATE robot_ipc)
    target_link_libraries(writer_struct PRIVATE robot_ipc)
    set_target_properties(reader_struct writer_struct PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/host_variable_struct"
    )
endif()
