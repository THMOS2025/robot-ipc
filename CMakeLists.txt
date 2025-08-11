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
    add_executable(shm_reader tests/shm_reader.c)
    target_link_libraries(shm_reader PRIVATE robot_ipc)

    add_executable(shm_writer tests/shm_writer.c)
    target_link_libraries(shm_writer PRIVATE robot_ipc)

    # Add warnings only for this target
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(shm_reader PRIVATE -Wall -Wextra -Wpedantic -g)
        target_compile_options(shm_writer PRIVATE -Wall -Wextra -Wpedantic -g)
    elseif(MSVC)
        target_compile_options(shm_reader PRIVATE /W4 /Zi)
        target_compile_options(shm_writer PRIVATE /W4 /Zi)
    endif()



    add_executable(delete_shm examples/delete_shm.c)
    target_link_libraries(delete_shm PRIVATE robot_ipc)

    target_include_directories(shm_reader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(shm_writer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(delete_shm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()
