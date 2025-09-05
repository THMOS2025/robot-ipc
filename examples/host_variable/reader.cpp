#include <iostream>
#include <thread>
#include <chrono>

#include <robot_ipc.hpp>


int main(int argc, char **argv) {
    RobotIPC::HostVariable<int> x("host_variable");

    std::cout << "Reader started" << std::endl;
    while(true) {
        int data = 0;
        x.read(data);
        std::cout << "data = " << data << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
