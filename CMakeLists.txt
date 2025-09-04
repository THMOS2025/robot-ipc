cmake_minimum_required(VERSION 3.10)
project(robot_ipc C CXX)

# options
option(BUILD_EXAMPLE    "Build the examples"        OFF)
option(BUILD_C          "Build the cpp interface"   ON)
option(BUILD_CPP        "Build the cpp interface"   ON)

# set default multi-thread 
if(NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    set(CMAKE_BUILD_PARALLEL_LEVEL 8)
endif()

# set default build type
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()


# search for c source files 
if(BUILD_C)
    file(GLOB C_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
endif()

# search for cpp source files
if(BUILD_CPP)
    file(GLOB CPP_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
endif()


# build the shared library
add_library(robot_ipc SHARED ${C_SRC_FILES} ${CPP_SRC_FILES})
target_include_directories(robot_ipc
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SORUCE_DIR}/src)
target_compile_features(robot_ipc PRIVATE cxx_std_17 c_std_11)


# whether to build example
if(BUILD_EXAMPLE)
    add_subdirectory(examples)
endif()
