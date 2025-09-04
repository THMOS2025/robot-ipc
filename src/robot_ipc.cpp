#include <cstring>
#include <iostream>
#include <stdexcept>

#include "host_variable.h"
#include "host_function_caller.h"
#include "host_function_receiver.h"

#include "robot_ipc.hpp"

namespace RobotIPC {

/* I know using reinterpret_cast disables safety check 
 * and is not regarded 'graceful' coe, but here we have
 * to isoloated the cpp interface and c interface while
 * sharing the same underlying. So that's it. */

/* Host variable */
template<typename T>
HostVariable<T>::HostVariable(const std::string& name) : name(name){
    this->p = reinterpret_cast<RobotIPC::host_variable>(\
              link_host_variable(this->name.c_str(), sizeof(T)));
    if(!this->p)
        throw std::runtime_error("Can not link host variable.");
}

template<typename T>
HostVariable<T>::~HostVariable() {
    unlink_host_variable(reinterpret_cast<::host_variable>(this->p), \
            this->name.c_str(), sizeof(T));
}

template<typename T>
int HostVariable<T>::write(const T& data) {
    return write_host_variable(reinterpret_cast<::host_variable>(this->p), \
            &data, sizeof(T));
}

template<typename T>
int HostVariable<T>::read(T& data) {
    return read_host_variable(reinterpret_cast<::host_variable>(this->p), \
            &data, sizeof(T));
}


/* Host function caller */
template<typename R, typename... Arg>
HostFunctionCaller<R, Arg...>::HostFunctionCaller(const std::string& name) {
    constexpr size_t sum_sz_args = (sizeof(Arg) + ...);
    this->args_buf = std::make_unique<char[]>(sum_sz_args);
    this->p = reinterpret_cast<RobotIPC::host_function_caller>(\
            link_host_function(name.c_str(), sum_sz_args, sizeof(R)));
}

template<typename R, typename... Arg>
HostFunctionCaller<R, Arg...>::~HostFunctionCaller() {
    unlink_host_function(reinterpret_cast<::host_function_caller>(this->p));
}

template<typename R, typename... Arg>
int HostFunctionCaller<R, Arg...>::operator()(Arg&&... args) {
    size_t tmp = 0;
    auto copier = [&](auto&& arg) {
        memcpy(this->args_buf.get() + tmp, &arg, sizeof(arg));
        tmp += sizeof(arg);
    };
    (copier(std::forward<Arg>(args)), ...);
    return call_host_function(\
            reinterpret_cast<::host_function_caller>(this->p), \
            this->args_buf.get());
}

template<typename R, typename... Arg>
int HostFunctionCaller<R, Arg...>::get_response(R& ret) {
    return get_response_host_function_caller(\
            reinterpret_cast<::host_function_caller>(this->p), \
            ret);
}


/* Host function dispatcher */
HostFunctionDispatcher::HostFunctionDispatcher(const size_t& num_func) {
    this->p = reinterpret_cast<RobotIPC::host_function_dispatcher>(\
            create_host_function_dispatcher(num_func));
}

HostFunctionDispatcher::~HostFunctionDispatcher() {
    delete_host_function_dispatcher(\
            reinterpret_cast<::host_function_dispatcher>(this->p));
}

template<typename R, typename... Arg>
int HostFunctionDispatcher::attach(\
        const std::string &name, \
        std::function<R(Arg...)> foo) {
    constexpr size_t sum_sz_args = (sizeof(Arg) + ...);
    return attach_host_function_dispatcher(\
            reinterpret_cast<::host_function_dispatcher>(this->p), \
            name.c_str(), foo, \
            sum_sz_args, sizeof(R));
}

int HostFunctionDispatcher::start() {
    return start_host_function_dispatcher(\
            reinterpret_cast<::host_function_dispatcher>(this->p));
}


};
