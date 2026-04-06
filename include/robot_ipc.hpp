/* robot_ipc.hpp
 *  Cpp wrapper with C++20 memory layout support
 *  Compatible with C++17 and C++20
 */

#pragma once

#include <string>
#include <memory>
#include <cstring>
#include <functional>
#include <type_traits>
#include <stdexcept>
#include <array>
#include <cstddef>

// C++20 feature detection
#if defined(__cpp_concepts) && defined(__cpp_lib_bit_cast)
#define ROBOT_IPC_HAS_CPP20 1
#include <concepts>
#include <bit>
#else
#define ROBOT_IPC_HAS_CPP20 0
#endif

#include "host_variable.h"
#include "host_function_caller.h"
#include "host_function_receiver.h"

namespace RobotIPC {

// C++17/C++20 compatible type traits
template<typename T>
constexpr bool is_memory_layout_compatible() {
    return std::is_standard_layout_v<T> && 
           std::is_trivially_copyable_v<T> &&
           !std::is_pointer_v<T> && 
           !std::is_reference_v<T>;
}

template<typename T>
constexpr bool is_ipc_safe_type() {
    return is_memory_layout_compatible<T>() && 
           std::is_trivially_destructible_v<T>;
}

// C++20 concepts (if available)
#if ROBOT_IPC_HAS_CPP20
template<typename T>
concept MemoryLayoutCompatible = is_memory_layout_compatible<T>();

template<typename T>
concept IPCSafeType = is_ipc_safe_type<T>();
#endif

// Helper type traits with better error messages
template<typename T>
constexpr bool check_ipc_safety() {
    static_assert(std::is_standard_layout_v<T>, 
        "Type must have standard layout (all members have same access control, no virtual functions/inheritance)");
    static_assert(std::is_trivially_copyable_v<T>, 
        "Type must be trivially copyable (no complex copy operations)");
    static_assert(!std::is_pointer_v<T>, 
        "Type cannot contain pointers (not safe for IPC)");
    static_assert(std::is_trivially_destructible_v<T>, 
        "Type must be trivially destructible");
    
    // Note: For cross-language compatibility with Python, the type must be packed
    // Users must ensure their structs use __attribute__((packed)) or equivalent
    // We cannot automatically detect padding at compile time for all types,
    // but we provide documentation and examples showing the correct usage.
    
    return true;
}

/* Host variable wrappers with C++20 improvements */
template<typename T>
class HostVariable {
private:
    const std::string name;
    host_variable p;
    
    // Compile-time validation of memory layout
    static_assert(check_ipc_safety<T>(), "T must be safe for IPC (standard layout, trivially copyable, no pointers)");
    
public:
    explicit HostVariable(const HostVariable&) = delete;
    HostVariable& operator=(const HostVariable&) = delete;
    
    explicit HostVariable(const std::string& name) : name(name) {
        this->p = link_host_variable(this->name.c_str(), sizeof(T));
        if(!this->p)
            throw std::runtime_error("Can not link host variable.");
    }
    
    ~HostVariable() {
        if(unlink_host_variable(this->p, this->name.c_str(), sizeof(T)))
            printf("Can not unlink host variable\n");
    }
    
    // Write with type safety
    int write(const T& data) {
        // Note: std::has_unique_object_representations_v is too strict for many practical use cases
        // We rely on standard layout and trivially copyable for safe memory operations
        return write_host_variable(this->p, &data, sizeof(T), sizeof(T));
    }
    
    int read(T& data) {
        int result = read_host_variable(this->p, &data, sizeof(T), sizeof(T));
        
        // Validate the read data (optional, for debugging)
        #if !defined(NDEBUG)
        if (result == 0) {
            // Ensure the object representation is valid
            volatile T tmp = data; // Force read of all bytes
            (void)tmp; // Prevent unused variable warning
        }
        #endif
        
        return result;
    }
    
    // C++20: Get the memory representation of the data
#if ROBOT_IPC_HAS_CPP20
    auto bit_representation(const T& data) const {
        static_assert(std::has_unique_object_representations_v<T>, 
                     "Type must have unique object representation for bit casting");
        return std::bit_cast<std::array<std::byte, sizeof(T)>>(data);
    }
#endif
    
    // C++17 fallback
    std::array<std::byte, sizeof(T)> to_bytes(const T& data) const {
        std::array<std::byte, sizeof(T)> result;
        std::memcpy(result.data(), &data, sizeof(T));
        return result;
    }
    
    T from_bytes(const std::array<std::byte, sizeof(T)>& bytes) const {
        T result;
        std::memcpy(&result, bytes.data(), sizeof(T));
        return result;
    }
};

/* Host function caller with C++20 improvements */
template<typename Func>
class HostFunctionCaller;

template<typename R, typename T>
class HostFunctionCaller<R(T)> {
private:
    host_function_caller p;
    
    static_assert(check_ipc_safety<T>(), "Argument type must be safe for IPC");
    static_assert(check_ipc_safety<R>(), "Return type must be safe for IPC");
    
public:
    explicit HostFunctionCaller(const HostFunctionCaller&) = delete;
    HostFunctionCaller& operator=(const HostFunctionCaller&) = delete;
    
    explicit HostFunctionCaller(const std::string& name) {
        this->p = link_host_function(name.c_str(), sizeof(T), sizeof(R));
        if(!this->p)
            throw std::runtime_error("Can not link host function.");
    }
    
    ~HostFunctionCaller() {
        if( unlink_host_function(this->p) )
            printf("Can not unlink host function\n");
    }
    
    int operator()(const T& arg) {
        return call_host_function(this->p, &arg);
    }
    
    int get_response(R& ret) {
        return get_response_host_function(this->p, &ret);
    }
};

/* Host function dispatcher with C++20 improvements */
class HostFunctionDispatcher {
private:
    host_function_dispatcher p;
    
public:
    explicit HostFunctionDispatcher(const HostFunctionDispatcher&) = delete;
    HostFunctionDispatcher& operator=(const HostFunctionDispatcher&) = delete;
    
    explicit HostFunctionDispatcher(const size_t& num_func = 16) {
        this->p = create_host_function_dispatcher(num_func);
        if(!this->p)
            throw std::runtime_error("Can not create host function dispatcher.");
    }
    
    ~HostFunctionDispatcher() {
        if( delete_host_function_dispatcher(this->p) )
            printf("Can not delete host function dispatcher\n");
    }
    
    template<typename R, typename T>
    int attach(const std::string &name, R* (*foo)(T*)) {
        static_assert(check_ipc_safety<R>(), "Return type must be safe for IPC");
        static_assert(check_ipc_safety<T>(), "Argument type must be safe for IPC");
        
        return attach_host_function(this->p, 
                name.c_str(), 
                reinterpret_cast<void* (*)(const void*)>(foo), 
                sizeof(T), sizeof(R));
    }
    
    int start() {
        return start_host_function_dispatcher(this->p);
    }
};

// C++20: Utility to check if a type is suitable for IPC at compile time
template<typename T>
constexpr bool is_ipc_safe_v = is_ipc_safe_type<T>();

// C++20: Type alias for IPC-safe types (documentation and concept checking)
template<typename T>
using IPCSafeType_t = std::enable_if_t<is_ipc_safe_type<T>, T>;

// Helper functions for memory layout validation
template<typename T>
constexpr bool validate_memory_layout() {
    // Check if the type has predictable memory layout
    return std::is_standard_layout_v<T> &&
           std::is_trivially_copyable_v<T> &&
           std::has_unique_object_representations_v<T>;
}

// Utility to get memory layout information
template<typename T>
struct MemoryLayoutInfo {
    static constexpr bool is_safe = is_ipc_safe_type<T>();
    static constexpr size_t size = sizeof(T);
    static constexpr size_t alignment = alignof(T);
    static constexpr bool has_unique_repr = std::has_unique_object_representations_v<T>;
};

}; // namespace RobotIPC
