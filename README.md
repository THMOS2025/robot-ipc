# Robot IPC: A Lightweight Inter-Process Communication Library

### Motivation

Traditional solutions for multi-process robotics, such as **ROS**, are often overly complex for simple projects and introduce unnecessary overhead. This library was created to address the shortcomings of ROS and similar frameworks.

- **Complexity:** ROS is a rigid framework that forces developers to structure code within its package system, which can lead to conflicts with other libraries.
- **Performance:** ROS sacrifices speed and low latency to support distributed communication across a local area network (LAN), features that are often not needed for a single robot's on-board computing.
- **Usability:** ROS can be confusing and problematic with alternating network connections (wired and wireless). For example, a system booted with a wired connection may lose topic visibility after the cable is unplugged, even if data should be handled locally.

Similarly, many DDS frameworks, like FastDDS, are designed for distributing data across a reliable industrial network. However, for many robotic applications, a publish-subscribe model isn't required. We simply need to read the latest data when necessary, rather than tracking every single update. This is like checking a weather app for the current forecast rather than getting a notification for every minor change.

* * *

### Concepts

This library introduces two core concepts for inter-process communication: `host_variable` and `host_function`. Just as a global variable or function can be accessed across different modules within a single process, these "host" objects can be accessed across different processes.

- host_variable  
    A `host_variable` is a piece of **shared memory** identified by a unique name (a string). It's managed internally, and a `host_variable` object (a `typedef` of a struct pointer) acts as the handle to this shared memory.
    
- host_function  
    A `host_function` is a function that can be called by another process. Arguments and results are transferred via **named pipes**. The process that defines the function runs a **dispatcher thread** that listens for incoming requests on these pipes.  
    Functions are called **asynchronously** within a single dispatcher.  
    Functions dispatched by different dispatchers run **concurrently** in different threads.
    

* * *

### Building the Library

This project uses **CMake** for its build system, making it easy to integrate into your existing project. Simply use `add_subdirectory` to include the main `CMakeLists.txt` file located in the root directory. **Static linking** is recommended for simplicity, but **dynamic linking** is also fully supported.

- BUILD_EXAMPLE  
    To build the examples included with the library, set the CMake option `BUILD_EXAMPLE` to `ON`.
- BUILD_C  
    Whether to build C interface into the .so file.
- BUILD_CPP  
    Whether to build C++ interface into the .so file.

* * *

### APIs & manual

Include "robot_ipc.h" for C style definitions and "robot_ipc.hpp" for C++ style definitions.

- HostVariable  
    **C style**
    
    - `typedef struct _s_host_variable host_variable`: host_variable is the handle of a host_variable.
    - `host_variable link_host_variable(const char* name, const size_t size)`  
        `name`: A string that identify the host_variable.  
        `size`: The size of the variable. Have to be the same **everywhere** you operate this variable strictly, in process or inter-process.  
        `return`: A handle of the host_variable. NULL if encounted error.
    - `int read_host_variable(host_variable p, void *buf, const size_t size)`  
        `p`: A valid host_variable handle to read from.  
        `buf`: A void\* pointer referring to a memory buf where the data is going to be copied to. Have to be larger than size.  
        `size`: The size of the variable, as previously mentioned.  
        `return`: 0 if success, otherwise the error code.
    - `int write_host_variable(host_varible p, const void *data, const size_t size)`  
        `p`: A valid host_variable to write to.  
        `buf`: The buffer of data to write into the variable.  
        `size`: The sizeof the variable, as mentioned.  
        `return`: 0 if success, otherwise the error code.
    
    **C++ style**
    
    - `template<typename T> class HostVariable`: The class wrapper in C++ style. T have to be a POD type.
        
        > ##### POD
        > 
        > POD, or Plain Old Data, is a type that has a fixed memory structure which can be dumped (or, serialized in python) into binary data using memcpy. In C++, a typically POD is a struct without any functions/method. Builtin types are POD, but containers in STL aren't.  
        > To transfer data between process, data structures have to be dumped first and should not contain process-related pointers ( otherwise, another process can only read the pointer itself but not the data it refers to ). So template type T have to be a POD here.
        
    - `HostVariable(const std::string& name)`: Initialize function  
        `name`: The string to identify the variable
        
    - `int .write(const T& data)`: Write into the variable  
        `data`: Reference to data.  
        `return`: 0 if success, otherwise error code.
        
    - `int .read(T& data)`: Read from variable  
        `data`: Reference to receive buffer.  
        `return`: 0 if success, otherwise error code.
        
- HostFunctionCaller  
	> ##### Note
	> Limited to the language, remote funciton has only one argument and can only return at most one response. Argument and response should be POD both.
    
    **C Style**
    
    - `typedef struct _s_host_function_caller* host_function_caller;` Works as a handle to call a remote funciton.
    - `host_function_caller link_host_funcition(const char* name, const size_t sz_arg, const size_t sz_ret);`       
        `name` Link to a remote function identified by name.      
        `sz_arg` is the size of the argument buffe.     
        `sz_ret` indicates the size of the return buffer.     
        `return` A handle (pointer) of a remote function caller object if success and NULL otherwise.
    - `int call_host_function(host_function_caller p, const void* arg);`      
        `p` The remote function handle       
        `arg` A pointer of a buffer with size sz_arg (The second parameter in the previous API).  
        `return` 0 if success and error code otherwise.
    - `int get_response_host_function(host_funcion_caller p, void *ret_buf)`    
        `p` The handle    
        `ret_buf` The buffer to store the response by remote function.
    
    **C++ Style**
    
    - `HostFunctionCaller<R(T)>(const std::string& name)`     
        `R` The return type of remote function    
        `T` The parameter type of remote function
    - `int .operator()(const T& arg)`: Call remote function     
        `arg` The argument to be passed     
        `return`   0 if success and error code otherwise
    - `int .get_response(R& ret)`     
        `ret`: The reference to a R object in which the response is going to be store. 
    

