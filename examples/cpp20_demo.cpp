#include <iostream>
#include <chrono>
#include "robot_ipc.hpp"

// Define a struct suitable for IPC
// IMPORTANT: Must be packed for cross-language compatibility with Python
struct __attribute__((packed)) SensorData {
    int64_t timestamp;
    double temperature;
    double humidity;
    int sensor_id;
    
    // No virtual functions, pointers, or complex types
    // __attribute__((packed)) ensures no padding for cross-language compatibility
};

int main() {
    std::cout << "=== Robot IPC C++20 Demo ===\n\n";
    
    // Compile-time validation of memory layout
    std::cout << "Memory Layout Info for SensorData:\n";
    std::cout << "  Is IPC safe: " << RobotIPC::MemoryLayoutInfo<SensorData>::is_safe << "\n";
    std::cout << "  Size: " << RobotIPC::MemoryLayoutInfo<SensorData>::size << " bytes\n";
    std::cout << "  Alignment: " << RobotIPC::MemoryLayoutInfo<SensorData>::alignment << " bytes\n";
    std::cout << "  Has unique representation: " << RobotIPC::MemoryLayoutInfo<SensorData>::has_unique_repr << "\n";
    
    // Check if the struct is packed (no padding)
    constexpr size_t expected_size = sizeof(int64_t) + sizeof(double) + sizeof(double) + sizeof(int);
    std::cout << "  Expected size (no padding): " << expected_size << " bytes\n";
    std::cout << "  Actual size: " << RobotIPC::MemoryLayoutInfo<SensorData>::size << " bytes\n";
    std::cout << "  Is packed: " << (RobotIPC::MemoryLayoutInfo<SensorData>::size == expected_size ? "Yes" : "No") << "\n\n";
    
    // Create host variable (shared memory)
    try {
        RobotIPC::HostVariable<SensorData> sensor_data("demo_sensor");
        std::cout << "Created shared memory variable 'demo_sensor'\n\n";
        
        // Writer process simulation
        std::cout << "--- Writer Simulation ---\n";
        SensorData write_data{};
        write_data.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        write_data.temperature = 23.5;
        write_data.humidity = 45.2;
        write_data.sensor_id = 1;
        
        std::cout << "Writing data:\n";
        std::cout << "  Timestamp: " << write_data.timestamp << "\n";
        std::cout << "  Temperature: " << write_data.temperature << "°C\n";
        std::cout << "  Humidity: " << write_data.humidity << "%\n";
        std::cout << "  Sensor ID: " << write_data.sensor_id << "\n\n";
        
        // Write to shared memory
        if (sensor_data.write(write_data) == 0) {
            std::cout << "✓ Data written successfully\n\n";
        } else {
            std::cout << "✗ Failed to write data\n";
            return 1;
        }
        
        // Reader process simulation
        std::cout << "--- Reader Simulation ---\n";
        SensorData read_data{};
        
        // Read from shared memory
        if (sensor_data.read(read_data) == 0) {
            std::cout << "✓ Data read successfully\n";
            std::cout << "Read data:\n";
            std::cout << "  Timestamp: " << read_data.timestamp << "\n";
            std::cout << "  Temperature: " << read_data.temperature << "°C\n";
            std::cout << "  Humidity: " << read_data.humidity << "%\n";
            std::cout << "  Sensor ID: " << read_data.sensor_id << "\n\n";
        } else {
            std::cout << "✗ Failed to read data\n";
            return 1;
        }
        
        // Demonstrate byte conversion (C++17 fallback)
        std::cout << "--- Byte Representation Demo ---\n";
        auto bytes = sensor_data.to_bytes(write_data);
        std::cout << "Data converted to " << bytes.size() << " bytes\n";
        
        // Convert back
        auto reconstructed = sensor_data.from_bytes(bytes);
        std::cout << "Reconstructed data temperature: " << reconstructed.temperature << "°C\n";
        std::cout << "Original matches reconstructed: " 
                  << (write_data.temperature == reconstructed.temperature ? "✓" : "✗") << "\n\n";
        
        std::cout << "=== Demo completed successfully! ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}