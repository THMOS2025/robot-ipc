#include <iostream>

#include <robot_ipc.hpp>


int main(int argc, char **argv) {
    RobotIPC::HostVariable<int> x("host_variable");
    x.write(100);
    std::cout << "Write data = 100" << std::endl;
    return 0;
}
