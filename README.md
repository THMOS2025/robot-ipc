# Robot IPC: A Lightweight Inter-Process Communication Library

### Motivation

Traditional solutions for multi-process robotics, such as **ROS**, are often overly complex for simple projects and introduce unnecessary overhead. This library was created to address the shortcomings of ROS and similar frameworks.

- **Complexity:** ROS is a rigid framework that forces developers to structure code within its package system, which can lead to conflicts with other libraries.
- **Performance:** ROS sacrifices speed and low latency to support distributed communication across a local area network (LAN), features that are often not needed for a single robot's on-board computing.
- **Usability:** ROS can be confusing and problematic with alternating network connections (wired and wireless). For example, a system booted with a wired connection may lose topic visibility after the cable is unplugged, even if data should be handled locally.

Similarly, many DDS frameworks, like FastDDS, are designed for distributing data across a reliable industrial network. However, for many robotic applications, a publish-subscribe model isn't required. We simply need to read the latest data when necessary, rather than tracking every single update. This is like checking a weather app for the current forecast rather than getting a notification for every minor change.

* * *

### Concepts & Usage

This library introduces two core concepts for inter-process communication: `host_variable` and `host_function`. Just as a global variable or function can be accessed across different modules within a single process, these "host" objects can be accessed across different processes.

- host_variable  
    A `host_variable` is a piece of **shared memory** identified by a unique name (a string). It's managed internally, and a `host_variable` object (a `typedef` of a struct pointer) acts as the handle to this shared memory.
    
- host_function  
    A `host_function` is a function that can be called by another process. Arguments and results are transferred via **named pipes**. The process that defines the function runs a **dispatcher thread** that listens for incoming requests on these pipes.  
    Functions are called **asynchronously** within a single dispatcher.  
    Functions dispatched by different dispatchers run **concurrently** in different threads.
    
See [apis.md](docs/apis.md) for detailed api definition and [python3-apis.md](docs/python3-apis.md) or [python3-apis-cn.md](docs/python3-apis-cn.md) for python interfaces. 

* * *

### Building the Library

This project uses **CMake** for its build system, making it easy to integrate into your existing project. Simply use `add_subdirectory` to include the main `CMakeLists.txt` file located in the root directory. **Static linking** is recommended for simplicity, but **dynamic linking** is also fully supported.

- BUILD_EXAMPLE  
    To build the examples included with the library, set the CMake option `BUILD_EXAMPLE` to `ON`.
- BUILD_C  
    Whether to build C interface into the .so file.
- BUILD_CPP  
    Whether to build C++ interface into the .so file.

