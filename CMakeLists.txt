cmake_minimum_required(VERSION 3.10)
project(robot_ipc C)

option(BUILD_EXAMPLE "Build the test suite" ON)
option(DEBUG_BUILD "Enable debug flags and symbols" OFF)

file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(robot_ipc SHARED ${SRC_FILES})

target_include_directories(robot_ipc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(robot_ipc PUBLIC pthread)

# C99
target_compile_features(robot_ipc PUBLIC c_std_99)

if(DEBUG_BUILD)
    # If the switch is ON, set the build type to Debug
    set(CMAKE_BUILD_TYPE Debug)

    # You can also add more specific debug flags here
    # For example, for GCC/Clang:
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options("-g" "-O0")
    endif()
else()
    # If the switch is OFF, set the build type to Release
    set(CMAKE_BUILD_TYPE Release)

    # Add optimization flags for a release build
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options("-O2")
    endif()
endif()


# examples
if(BUILD_EXAMPLE)
    add_executable(reader examples/reader.c)
    target_link_libraries(reader PRIVATE robot_ipc)

    add_executable(writer examples/writer.c)
    target_link_libraries(writer PRIVATE robot_ipc)

    target_include_directories(reader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(writer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()
