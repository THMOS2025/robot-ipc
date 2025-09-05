/* robot_ipc.hpp
 *  Cpp wrapper
 */

#pragma once

#include <string>
#include <memory>
#include <cstring>
#include <functional>

#include "host_variable.h"
#include "host_function_caller.h"
#include "host_function_receiver.h"

namespace RobotIPC {


/* Host variable wrappers */
template<typename T>
class HostVariable {
private:
    const std::string name;
    host_variable p;
public:
    explicit HostVariable(const HostVariable&) = delete;
    HostVariable& operator=(const HostVariable&) = delete;
    explicit HostVariable(const std::string& name) : name(name) {
        this->p = link_host_variable(this->name.c_str(), sizeof(T));
        if(!this->p)
            throw std::runtime_error("Can not link host variable.");
    }
    ~HostVariable() {
        unlink_host_variable(this->p, this->name.c_str(), sizeof(T));
    }
    int write(const T& data) {
        return write_host_variable(this->p, &data, sizeof(T));
    }
    int read(T& data) {
        return read_host_variable(this->p, &data, sizeof(T));
    }
};


/* Host function caller */
// Primary template declaration, to be specialized below
template<typename Func>
class HostFunctionCaller;

// Partial specialization for function types (e.g., int(int, int))
template<typename R, typename T>
class HostFunctionCaller<R(T)> {
private:
    host_function_caller p;
public:
    explicit HostFunctionCaller(const HostFunctionCaller&) = delete;
    HostFunctionCaller& operator=(const HostFunctionCaller&) = delete;
    explicit HostFunctionCaller(const std::string& name) {
        this->p = link_host_function(name.c_str(), sizeof(T), sizeof(R));
    }
    ~HostFunctionCaller() {
        unlink_host_function(this->p);
    }
    int operator()(const T& arg) {
        return call_host_function(this->p, &arg);
    }
    int get_response(R& ret) {
        return get_response_host_function(this->p, &ret);
    }
};


/* Host function dispatcher */
class HostFunctionDispatcher {
private:
    host_function_dispatcher p;
public:
    explicit HostFunctionDispatcher(const HostFunctionDispatcher&) = delete;
    HostFunctionDispatcher& operator=(const HostFunctionDispatcher&) = delete;
    explicit HostFunctionDispatcher(const size_t& num_func = 16) {
        this->p = create_host_function_dispatcher(num_func);
    }
    ~HostFunctionDispatcher() {
        delete_host_function_dispatcher(this->p);
    }
    template<typename R, typename T>
    int attach(const std::string &name, R* (*foo)(T*)) {
        return attach_host_function(this->p, \
                name.c_str(), \
                reinterpret_cast<void* (*)(const void*)>(foo), 
                sizeof(T), sizeof(R));
    }
    int start() {
        return start_host_function_dispatcher(this->p);
    }
};


};
