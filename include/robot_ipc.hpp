/* robot_ipc.hpp
 *  Cpp wrapper
 */

#pragma once

#include <string>
#include <memory>
#include <functional>

namespace RobotIPC {


/* Host variable wrappers */
struct _s_host_variable;
using host_variable = struct _s_host_variable*;

template<typename T>
class HostVariable {
private:
    const std::string name;
    struct _s_host_variable* p;
public:
    HostVariable(const std::string& name);
    explicit HostVariable(const HostVariable&) = delete;
    ~HostVariable();
    HostVariable& operator=(const HostVariable&) = delete;
    int write(const T& data);
    int read(T& data);
};


/* Host function caller */
struct _s_host_function_caller;
using host_function_caller = struct _s_host_function_caller*;

template<typename R, typename... Arg>
class HostFunctionCaller {
private:
    std::unique_ptr<char[]> args_buf;
    struct _s_host_function_caller* p;
public:
    HostFunctionCaller(const HostFunctionCaller&) = delete;
    explicit HostFunctionCaller(const std::string& name);
    ~HostFunctionCaller();
    HostFunctionCaller& operator=(const HostFunctionCaller&) = delete;
    int operator()(Arg&&... args);
    int get_response(R& ret);
};


/* Host function dispatcher */
struct _s_host_functino_dispatcher;
using host_function_dispatcher = struct _s_host_function_dispatcher*;

class HostFunctionDispatcher {
private:
    struct _s_host_function_dispatcher* p;
public:
    HostFunctionDispatcher(const HostFunctionDispatcher&) = delete;
    explicit HostFunctionDispatcher(const size_t& num_func = 16);
    ~HostFunctionDispatcher();
    HostFunctionDispatcher& operator=(const HostFunctionDispatcher&) = delete;
    template<typename R, typename... Arg>
    int attach(const std::string &name, std::function<R(Arg...)> foo);
    int start();
};


};
