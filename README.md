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

### Cross-Language Compatibility

For cross-language IPC (especially with Python), it's critical that your structs have **no padding** to ensure consistent memory layout across different languages and compilers.

#### **Requirements for Cross-Language Structs**

1. **Use packed structs** to eliminate padding:
   ```cpp
   // GOOD: No padding for cross-language compatibility
   struct __attribute__((packed)) SensorData {
       int64_t timestamp;
       double temperature;  
       double humidity;
       int sensor_id;
       // Total: 28 bytes (no padding)
   };
   
   // Alternative: Using pragma pack (works on MSVC/GCC/clang)
   #pragma pack(push, 1)
   struct RobotStatus {
       int32_t x;
       int32_t y;  
       float battery;
       uint8_t status;
       // Total: 13 bytes (no padding)
   };
   #pragma pack(pop)
   ```

2. **AVOID unpacked structs** (they have compiler-specific padding):
   ```cpp
   // BAD: Has padding, not suitable for cross-language IPC
   struct BadSensorData {
       int64_t timestamp;      // 8 bytes
       double temperature;     // 8 bytes
       double humidity;        // 8 bytes
       int sensor_id;          // 4 bytes
       // Compiler may add 4 bytes padding for alignment
       // Total: 32 bytes (with padding)
   };
   ```

3. **Use fixed-size integer types**:
   ```cpp
   #include <cstdint>
   
   struct __attribute__((packed)) NetworkData {
       int32_t value1;     // Fixed 4 bytes
       uint8_t flags;      // Fixed 1 byte
       uint16_t checksum;  // Fixed 2 bytes
   };
   ```

#### **Python ctypes Compatibility**

When using Python ctypes, match the packed C++ struct exactly:

```python
import ctypes

class SensorData(ctypes.Structure):
    _pack_ = 1  # Equivalent to __attribute__((packed))
    _fields_ = [
        ('timestamp', ctypes.c_int64),
        ('temperature', ctypes.c_double),
        ('humidity', ctypes.c_double),
        ('sensor_id', ctypes.c_int),
    ]

class RobotStatus(ctypes.Structure):
    _pack_ = 1  # Packed struct
    _fields_ = [
        ('x', ctypes.c_int32),
        ('y', ctypes.c_int32),
        ('battery', ctypes.c_float),
        ('status', ctypes.c_uint8),
    ]
```

#### **C++20 Type Safety**

The C++20 wrapper provides compile-time validation:
```cpp
#include "robot_ipc.hpp"

struct __attribute__((packed)) MyData { /* ... */ };

// Compile-time validation
static_assert(RobotIPC::MemoryLayoutInfo<MyData>::is_safe, 
    "MyData must be safe for IPC");

// Size validation (ensure no padding)
static_assert(RobotIPC::MemoryLayoutInfo<MyData>::size == 
    sizeof(int64_t) + sizeof(double) + sizeof(double) + sizeof(int),
    "MyData must be packed (no padding)");
```

#### **Testing Your Structs**

Run the cross-language example to verify your structs:
```bash
# Build and run the cross-language struct examples
mkdir build && cd build
cmake -DBUILD_EXAMPLES=ON ..
make
./examples/cross_language_structs
```

* * *

### Building the Library

This project uses **CMake** for its build system, making it easy to integrate into your existing project. Simply use `add_subdirectory` to include the main `CMakeLists.txt` file located in the root directory. **Static linking** is recommended for simplicity, but **dynamic linking** is also fully supported.

- BUILD_EXAMPLE  
    To build the examples included with the library, set the CMake option `BUILD_EXAMPLE` to `ON`.
- BUILD_C  
    Whether to build C interface into the .so file.
- BUILD_CPP  
    Whether to build C++ interface into the .so file.

#### Example: Build with Examples

To build the library and the example programs, run the following commands from your project root:

```bash
mkdir build
cd build
cmake -DBUILD_EXAMPLE=ON ..
make
```

* * * 

### Python Installation 

Install the Python binding after building the shared library as above:

```bash
cd robot_ipc
pip install .
```

* * *

### Performance

The Robot IPC library is designed for exceptional performance in local inter-process communication scenarios. Below are benchmark results from automated testing:

#### Test Environment
- **CPU:** AMD Ryzen 9 7950X3D 16-Core Processor
- **Architecture:** x86_64
- **Kernel:** Linux 6.19.10-arch1-1
- **Compiler:** g++ (GCC) 15.2.1
- **Test Payload:** 1024 bytes

#### Benchmark Results

**Host Variable (Shared Memory)**
- **Average latency:** 0.149 μs (microseconds)
- **Standard deviation:** 0.177 μs
- **Typical range:** 0.12-0.19 μs
- **Implementation:** Direct shared memory with atomic operations and lock-free multi-buffer design

**Host Function (Named Pipes)**
- **Average latency:** 2.16 μs (microseconds)
- **Standard deviation:** 1.33 μs
- **Typical range:** 0.8-6.2 μs
- **Implementation:** Named pipes with blocking I/O and multi-threaded dispatchers

#### Performance Analysis

The library achieves **microsecond-level latencies**, making it significantly faster than network-based solutions like ROS (which typically operate in millisecond ranges):

- **Host Variables** provide near memory-access speeds, ideal for high-frequency data sharing
- **Host Functions** offer low-latency cross-process function calls
- **Consistent performance** with low standard deviation indicates stable, predictable timing
- **Optimized for local robotics** - eliminates network stack overhead for on-board computing

These performance characteristics make Robot IPC an excellent choice for real-time robotics applications requiring fast, deterministic inter-process communication.

* * *

### CI / CD

GitHub Actions is configured in `.github/workflows/ci.yml` with:

- **Build + smoke tests:** Builds library and examples (`BUILD_EXAMPLE=ON`), then runs automated smoke tests based on your examples (`scripts/ci/run_example_smoke_tests.sh`).
- **Style test:** Runs `clang-format` checks for changed C/C++ files (`scripts/ci/check_clang_format.sh`).
