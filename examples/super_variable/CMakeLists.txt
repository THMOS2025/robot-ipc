add_executable(reader reader.c)
add_executable(writer writer.c)
target_link_libraries(reader PRIVATE robot_ipc)
target_link_libraries(writer PRIVATE robot_ipc)
set_target_properties(reader writer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/super_variable"
)

