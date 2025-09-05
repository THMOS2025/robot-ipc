#include <iostream>
#include <string>
#include <random>
#include <thread>

#include <robot_ipc.hpp>

int main(int argc, char **argv) {
    std::cout << "HostFunctionCaller" << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0x80000000, 0x7fffffff);

    RobotIPC::HostFunctionCaller<int(int)> caller("host_function");
    std::cout << "HostFunctionCaller initialized" << std::endl;

    int ran = distrib(gen);
    caller(ran);
    std::cout << "Call host funciton with args = " << ran << std::endl;
    
    int ret;
    caller.get_response(ret);
    std::cout << "Return: " << ret << std::endl;

    return 0;
}
