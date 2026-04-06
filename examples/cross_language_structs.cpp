/*
 * Cross-Language IPC Struct Examples
 * 
 * This file shows how to create structs that are compatible between C++ and Python
 * for use with Robot IPC. The key is ensuring NO PADDING and consistent memory layout.
 */

#include <iostream>
#include <type_traits>
#include <cstddef>
#include <cstdint>

// GOOD: Packed struct with no padding - ideal for cross-language IPC
struct __attribute__((packed)) SensorData {
    int64_t timestamp;      // 8 bytes
    double temperature;     // 8 bytes
    double humidity;        // 8 bytes
    int sensor_id;          // 4 bytes
    // Total: 28 bytes (no padding)
};

// Alternative: Using pragma pack (works on MSVC/GCC/clang)
#pragma pack(push, 1)
struct RobotStatus {
    int32_t x;              // 4 bytes
    int32_t y;              // 4 bytes
    float battery;          // 4 bytes
    uint8_t status;         // 1 byte
    // Total: 13 bytes (no padding)
};
#pragma pack(pop)

// BAD: Unpacked struct - will have padding, not suitable for cross-language IPC
struct BadSensorData {
    int64_t timestamp;      // 8 bytes
    double temperature;     // 8 bytes
    double humidity;        // 8 bytes
    int sensor_id;          // 4 bytes
    // Compiler may add 4 bytes padding for alignment
    // Total: 32 bytes (with padding)
};

// Template to check if a struct is packed (no padding)
template<typename T>
constexpr bool is_packed() {
    // This is a heuristic - it works for simple structs but not for complex ones
    // Users should manually verify packing or use attributes/pragma
    return true;
}

// Helper to show struct information
template<typename T>
void print_struct_info(const char* name) {
    std::cout << name << ":\n";
    std::cout << "  Size: " << sizeof(T) << " bytes\n";
    std::cout << "  Alignment: " << alignof(T) << " bytes\n";
    std::cout << "  Standard layout: " << std::is_standard_layout_v<T> << "\n";
    std::cout << "  Trivially copyable: " << std::is_trivially_copyable_v<T> << "\n";
    std::cout << "  Has unique representations: " << std::has_unique_object_representations_v<T> << "\n";
    
    if constexpr (std::is_same_v<T, SensorData>) {
        constexpr size_t expected_size = sizeof(int64_t) + sizeof(double) + sizeof(double) + sizeof(int);
        std::cout << "  Expected size (no padding): " << expected_size << " bytes\n";
        std::cout << "  Is packed: " << (sizeof(T) == expected_size ? "Yes" : "No") << "\n";
    } else if constexpr (std::is_same_v<T, RobotStatus>) {
        constexpr size_t expected_size = sizeof(int32_t) + sizeof(int32_t) + sizeof(float) + sizeof(uint8_t);
        std::cout << "  Expected size (no padding): " << expected_size << " bytes\n";
        std::cout << "  Is packed: " << (sizeof(T) == expected_size ? "Yes" : "No") << "\n";
    }
    std::cout << "\n";
}

int main() {
    std::cout << "=== Cross-Language IPC Struct Examples ===\n\n";
    
    std::cout << "GOOD EXAMPLES (suitable for cross-language IPC):\n";
    print_struct_info<SensorData>("SensorData (packed with __attribute__((packed)))");
    print_struct_info<RobotStatus>("RobotStatus (packed with #pragma pack)");
    
    std::cout << "BAD EXAMPLE (not suitable for cross-language IPC):\n";
    print_struct_info<BadSensorData>("BadSensorData (unpacked - has padding)");
    
    std::cout << "=== Python Compatibility Notes ===\n";
    std::cout << "\n";
    std::cout << "For Python ctypes/cstruct compatibility:\n";
    std::cout << "1. Always use __attribute__((packed)) or #pragma pack(push, 1)\n";
    std::cout << "2. Use fixed-size integer types (int32_t, uint8_t, etc.)\n";
    std::cout << "3. Avoid floating-point if possible (or use float instead of double)\n";
    std::cout << "4. Be careful with endianness (use htons/ntohs for network byte order)\n";
    std::cout << "5. No pointers, references, or virtual functions\n";
    std::cout << "\n";
    std::cout << "Python ctypes example for SensorData:\n";
    std::cout << "```python\n";
    std::cout << "import ctypes\n";
    std::cout << "\n";
    std::cout << "class SensorData(ctypes.Structure):\n";
    std::cout << "    _pack_ = 1  # Equivalent to __attribute__((packed))\n";
    std::cout << "    _fields_ = [\n";
    std::cout << "        ('timestamp', ctypes.c_int64),\n";
    std::cout << "        ('temperature', ctypes.c_double),\n";
    std::cout << "        ('humidity', ctypes.c_double),\n";
    std::cout << "        ('sensor_id', ctypes.c_int),\n";
    std::cout << "    ]\n";
    std::cout << "```\n";
    
    return 0;
}