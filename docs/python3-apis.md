# Python APIs

Python APIs are different from C/C++ since PyObjects have different memory layout in the python describer. The both sides have to be python for the same host_variable/host_function unless: a. you make the pickled memory layout strictly the same with C struct; b. you handle it within your C code.

Python is flexible without a fixed memory layout length, even with pickle. However, the underlying C library is designed for C structs which has a fixed size. So, in python APIs, you may set the max size for each component and by default, they're 4096 bytes (4KB).  If the data pickled has a length that exceed the max size you set, the library raises an exception.But please remember that for a specific shared object, all users of this shared object must set the same max_size parameter (different max_sizes can be different).

### Installation

1.  Clone this project
2.  `mkdir build && cd build`
3.  `cmake .. && sudo make install -j4`
4.  Copy (or symbol link) file python3/robot_ipc.py to your project
5.  Make sure `ctypes` and `pickle` are installed in your python environment. They are installed by default.

* * *

### Host Variable

- `class HostVariable(name: str, max_size: int = 4096)`
    - name: A string that identify the variable
    - max_size: The size of the variable. The smaller the size is, the faster it will be. But your python objects have to be smaller than the max size after pickled.
- `HostVariable.data`
    - Take it as a simple local python variable. Whenever you write to it, the data will be synced host-wide.

> ### Note - the lock
> 
> You should avoid terminate the program (ctrl + c or sigkill 9) when the shared variable is reading or writing. Since the 'lock' implementing by a manually set byte, terminating when the 'lock' is acquired but not released results in a dead-lock of the slot. Although there's 14 slots by default and the chance of dead-locking them all is too tiny to be considered, the variable will stuck when it happens until you remove the shared memory and restart every process using it.  In general circumstance, this process is super fast that there's a very tiny possibility to be terminated while reading or writing. But if you update the variable at a extremely high frequency ( at least 10kHz, depends on your hardware ) and terminate and restart it consequently, you get a considerable chance to dead lock one slot. So, handle ctrl+c signal as gracefully as you can.
> 
> If you are on a debug stage and don't care much about the latency, you can build ( and install ) the dynamic library in DEBUG mode, in which the ctrl-C signal is disabled before locking and enabled after releasing the lock.

### Host Function Caller

- `class HostFunctionCaller(name: str, max_sz_args: int = 4096, max_sz_ret: int = 4096)`
    - name: A string that identity the variable
    - max_sz_args: maximum size of arguments passed ( after pickled )
    - max_sz_ret: maximum size of response returned ( after pickled )
- `__call__(*args, **kwargs)`: Just call the instance as a normal function
- `.get_response()`: Wait for the response and return it.

> ### Note - the response
> 
> Make sure your remote function actually return something (other than `None`) if you call `get_response()`. Otherwise, your process will stuck here. And if you do return something in your callback function, remember to `get_response()` or the tunnel stuck after the buffer is full. 
> 
> There are no response in these circumstance: 
> 
> - The `max_sz_ret` is set to 0.
> - Your callback function returns `None` explicitly. 
> - Your callback function doesn't return 

### Host Function Dispatcher

- `class HostFunctionDispatcher(max_func_count: int)`
    - max_func_count: How many functions you are going to add into this dispatcher.
- `.add(name: str, foo: function, max_sz_args = 4096, max_sz_ret = 4096)`
    - name: The identify of a host function.
    - foo: The callback function.
    - max_sz_arg: maximum size of arguments passed ( after pickled )
    - max_sz_ret: maximum size of response returned ( after pickled )

> ### Warning
> 
> Make sure your max_sz_arg and max_sz_ret match what you set at the caller side. Otherwise this tunnel stop working.

- `.start()`
    - Start the dispatcher as a daemon thread. 
    - Able to add another function (`.add()`) after started, but not full tested and may fail. 
- `__del()__`
    - The dispatcher stops working when the instance is garbage recollected.
