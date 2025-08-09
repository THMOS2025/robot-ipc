cmake_minimum_required(VERSION 3.10)
project(robot_ipc C)

file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(robot_ipc SHARED ${SRC_FILES})

target_include_directories(robot_ipc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(robot_ipc PUBLIC pthread)

# C99
target_compile_features(robot_ipc PUBLIC c_std_99)