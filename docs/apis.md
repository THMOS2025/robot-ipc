# APIs

The **robot_ipc** library provides a straightforward way to handle inter-process communication (IPC) through shared variables and remote function calls. You can include **robot_ipc.h** for C-style definitions or **robot_ipc.hpp** for C++-style definitions.

### Host Variable

Host vairable works in a symmatric way. Writer and reader shared the same apis. Host variales are exactly a memory shared by multiple process with data structures to maintain its integrity and allowance of multiple modification. 

Note that because host variables are underlying memory blocks, a POD structure must be used. A POD (Plain Old Data) type has a fixed memory layout, allowing its data to be directly copied (serialized) into a binary buffer using functions like `memcpy`. In C++, this typically means a `struct` or `class` without virtual functions, constructors, or other complex features. Standard C++ types (e.g., `int`, `float`) and simple `structs` are POD, but STL containers (e.g., `std::vector`, `std::string`) are not. Inter-process data transfer requires POD types to avoid issues with memory pointers that are only valid within a single process. Please pay extra attention to this when you are using python where memory layout aren't exposed. 

#### C-Style

* `typedef struct _s_host_variable* host_variable;`
    * **Description**: A handle representing a `host_variable`.
* `host_variable link_host_variable(const char* name, const size_t size)`
    * **`name`**: A unique string that identifies the variable.
    * **`size`**: The size of the variable in bytes. This must be the **same** across all processes interacting with this variable.
    * **`return`**: A valid `host_variable` handle, or `NULL` on error.
* `int read_host_variable(host_variable p, void *buf, const size_t size, const size_t sop_size)`
    * **`p`**: A valid `host_variable` handle to read from.
    * **`buf`**: A pointer to a memory buffer where the data will be copied. The buffer must be at least as large as `size`.
    * **`size`**: The size of the variable, which must match the size used during `link_host_variable`.
    * **`op_size`**: Indicate how many bytes are really write to buffer. Suitable for non-fixed length data structure.
    * **`return`**: `0` on success, or an error code otherwise.
* `int write_host_variable(host_variable p, const void *data, const size_t size, const size_t op_size)`
    * **`p`**: A valid `host_variable` handle to write to.
    * **`data`**: A pointer to the data buffer to be written.
    * **`size`**: The size of the variable, which must match the size used during `link_host_variable`.
    * **`op_size`**: Indicate how many bytes are really write to buffer. Suitable for non-fixed length data structure.
    * **`return`**: `0` on success, or an error code otherwise.
* `int unlink_host_variable(host_variable p, const char* name, const size_t size)`
    * **Description**: Unlink a host variable  
    * **`p`**: The handle
    * **`name`**: The name of the host variable
    * **`size`**: The size.

#### C++-Style

* `template<typename T> class HostVariable`
    * **Description**: A C++ class wrapper for `HostVariable`. The template type `T` must be a **Plain Old Data (POD)** type.
    * **POD Explained**: 
* `HostVariable(const std::string& name)`
    * **Description**: Initializes the `HostVariable` object.
    * **`name`**: The unique string identifier for the variable.
* `int .write(const T& data)`
    * **Description**: Writes data to the variable.
    * **`data`**: A constant reference to the data to be written.
    * **`return`**: `0` on success, or an error code otherwise.
* `int .read(T& data)`
    * **Description**: Reads data from the variable.
    * **`data`**: A reference to the buffer where the read data will be stored.
    * **`return`**: `0` on success, or an error code otherwise.

---

### Host Function Caller

This component allows you to call a function in a remote process. Due to language limitations, a remote function can only accept one argument and return a single response, and both the argument and return types must be POD.

#### C-Style

* `typedef struct _s_host_function_caller* host_function_caller;`
    * **Description**: A handle used to call a remote function.
* `host_function_caller link_host_function(const char* name, const size_t sz_arg, const size_t sz_ret)`
    * **`name`**: The unique string identifier of the remote function to link to.
    * **`sz_arg`**: The size of the argument buffer in bytes.
    * **`sz_ret`**: The size of the return buffer in bytes.
    * **`return`**: A handle to the `host_function_caller` object on success, or `NULL` on error.
* `int call_host_function(host_function_caller p, const void* arg)`
    * **Description**: Calls the remote function. This call is non-blocking. To get the result, you must use `get_response_host_function`.
    * **`p`**: The `host_function_caller` handle.
    * **`arg`**: A pointer to the argument buffer, with a size of `sz_arg`.
    * **`return`**: `0` on success, or an error code otherwise.
* `int get_response_host_function(host_function_caller p, void *ret_buf)`
    * **Description**: Retrieves the response from a previous `call_host_function`. This call may block until the response is ready.
    * **`p`**: The `host_function_caller` handle.
    * **`ret_buf`**: The buffer where the remote function's return data will be stored. The buffer must be large enough to hold `sz_ret` bytes.
    * **`return`**: `0` on success, or an error code otherwise.

#### C++-Style

* `template<typename R, typename T> HostFunctionCaller(const std::string& name)`
    * **Description**: A C++ class that simplifies remote function calls.
    * **`R`**: The return type of the remote function.
    * **`T`**: The argument type of the remote function.
    * **`name`**: The unique string identifier for the remote function.
* `int .operator()(const T& arg)`
    * **Description**: A convenient operator to call the remote function. This call is non-blocking. Use `get_response` to retrieve the result.
    * **`arg`**: The argument to be passed to the remote function.
    * **`return`**: `0` on success, or an error code otherwise.
* `int .get_response(R& ret)`
    * **Description**: Retrieves the response from the remote function. This call may block until the response is ready.
    * **`ret`**: A reference to an object of type `R` where the return value will be stored.
    * **`return`**: `0` on success, or an error code otherwise.

---

### Host Function Dispatcher

A host function dispatcher receive from host function caller and exactly call the function. A dispatcher can schedule multiple functions, but they can only be executed in parallel; meanwhile different dispatcher are running parallelly. 

Limited to language C, a function to be dispatched has to be within the same signature: ```void* foo(const void *arg)```. Pointer arg refers to a read-only memory representing the arguments given by caller, and you may return a pointer refers to a **static** memory storing the response. This memory block have to be readable after your function exited, so the best way to code it is using a static variable. Keep in mind that do not malloc and return its pointer since this memory will never be free and will cause memory leakage. 
Or, you can just return NULL if you have nothing to response. 

The size of argument and reponse (0 if non-exist) buffer have to be strictly the same at caller and dispatcher, otherwise the pipe will stuck and the function will never be executed. 

##### C-Style
* ```typdef struct _s_host_function_dispatcher* host_function_dispatcher```
	* **Description** The handle of a dispatcher. 
* ```host_function_dispatcher create_host_function_dispatcher(const size_t n)``` 
	* **``n``**:  The (maximum) number of functions to be dispatched.
* ```int delete_host_function_dispatcher(host_function_dispatcher p)```
	* **``p``** The handle of dispatcher to be delete. Notice that deleting it will stop the background daemon. This function may stuck if there is a current function executing. 
	* ``return``: 0 if success and error code otherwise
* ```int attach_host_function(host_function_dispatcher p, const char *name, host_function foo, const size_t sz_arg, const size_t sz_ret)```
	* **``p``** The handle of dispatcher to be attached to
	* **``name``** A string to identify a host_function. 
	* **``foo``** The pointer of your function. The signature have to be same as described above. 
	* **``sz_arg``** Size of argument.
	* **``sz_ret``** Size of response. 
	* ``return`` 0 if success and error code otherwise. 
* ```int start_host_function_dispatcher(host_function_dispatcher p)```
	* **Description**: Run the dispatcher in background. 
	* **``p``** The handle of dispatcher. 
	* **``return``** 0 if success and error code otherwise. 

##### C++-Style
* ```HostFunctionDispatcher(const size_t& num_func = 16)```
	* **Description**: The construct function of the class. num_func is the maximum number of functions to be dispatched by this instance. 
* ```template<typename R, typename T> int .attach(const std::string &name, R* (*foo)(T*))```
	* **``R``** The return type. 
	* **``T``** The argument type. 
	* **``name``** A string that identify the host function.
	* **``foo``** The function to be attached. 
	* ``return`` 0 if success and error code otherwise. 
* ```int .start()```
	* **Description** Run the dispatcher in background.
	* ``return`` 0 if success and error code otherwise. 

---

### Shared Memory Management
The robot_ipc library uses POSIX shared memory objects (typically under ```/dev/shm/```) to enable fast inter-process communication. These shared memory objects are not automatically deleted when your program exits—they must be manually unlinked (deleted) to avoid resource leaks. If you repeatedly run your program without cleanup, you may eventually run into system limits on the number of shared memory objects.

#### How to View Shared Memory Variables
To see which shared memory objects have been created (including those by robot_ipc), you can use the following command in your terminal to show shared memery names:
```bash
ls -lh /dev/shm/
```

To delete it manually:
```bash
rm /dev/shm/my_variable
```