#include <iostream>
#include <string>
#include <thread>

#include <robot_ipc.hpp>

int* foo(int *data) {
    std::cout << "foo called: data = " << *data << " ";
    static int ret = *data - 1;
    std::cout << "return = " << ret << std::endl;
    return &ret;
    // Or, disable returning by return nullptr
}

int main(int argc, char **argv) {
    std::cout << "host function receiver" << std::endl;

    RobotIPC::HostFunctionDispatcher dispatcher;
    std::cout << "HostFunctionDispatcher created" << std::endl;

    dispatcher.attach("host_function", foo);
    std::cout << "attach foo to HostFunctionDispatcher" << std::endl;

    dispatcher.start();
    std::cout << "HostFunctionDispatcher started" << std::endl;

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(10000));
    return 0;
}
