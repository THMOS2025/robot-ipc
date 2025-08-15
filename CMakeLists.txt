cmake_minimum_required(VERSION 3.10)
project(robot_ipc C)


option(BUILD_EXAMPLE "Build the examples" ON)
option(DEBUG_BUILD "Enable debug flags and symbols" OFF)

# multi-thread building
if(NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    set(CMAKE_BUILD_PARALLEL_LEVEL 8)
endif()


file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_library(robot_ipc SHARED ${SRC_FILES})
target_include_directories(robot_ipc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(robot_ipc PUBLIC pthread)
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


if(BUILD_EXAMPLE)
    add_subdirectory(examples)
endif()
