#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>

#include "host_function_caller.h"
#include "host_function_receiver.h"
#include "host_variable.h"

namespace py = pybind11;

namespace {

class PyHostVariable {
public:
    PyHostVariable(const std::string &name, size_t size)
        : name_(name), size_(size), buffer_(size) {
        handle_ = link_host_variable(name_.c_str(), size_);
        if (!handle_) {
            throw std::runtime_error("failed to link host variable: " + name_);
        }
    }

    ~PyHostVariable() {
        if (handle_) {
            unlink_host_variable(handle_, name_.c_str(), size_);
            handle_ = nullptr;
        }
    }

    PyHostVariable(const PyHostVariable &) = delete;
    PyHostVariable &operator=(const PyHostVariable &) = delete;

    void write(py::bytes payload, size_t op_size = 0) {
        const std::string data = payload;
        const size_t effective_size = op_size == 0 ? data.size() : op_size;
        if (effective_size > size_) {
            throw std::runtime_error("payload exceeds host variable size");
        }
        if (data.size() < effective_size) {
            throw std::runtime_error("payload shorter than op_size");
        }

        const int ret = write_host_variable(
            handle_,
            static_cast<const void *>(data.data()),
            size_,
            effective_size
        );
        if (ret != 0) {
            throw std::runtime_error("failed to write host variable");
        }
    }

    py::bytes read(size_t op_size = 0) {
        const size_t effective_size = op_size == 0 ? size_ : op_size;
        if (effective_size > size_) {
            throw std::runtime_error("op_size exceeds host variable size");
        }

        const int ret = read_host_variable(
            handle_,
            static_cast<void *>(buffer_.data()),
            size_,
            effective_size
        );
        if (ret != 0) {
            throw std::runtime_error("failed to read host variable");
        }

        return py::bytes(buffer_.data(), static_cast<ssize_t>(effective_size));
    }

    size_t size() const {
        return size_;
    }

private:
    std::string name_;
    size_t size_;
    host_variable handle_ = nullptr;
    std::vector<char> buffer_;
};

class PyHostFunctionCaller {
public:
    PyHostFunctionCaller(const std::string &name, size_t arg_size, size_t ret_size)
        : name_(name), arg_size_(arg_size), ret_size_(ret_size), buffer_(std::max(arg_size, ret_size), 0) {
        handle_ = link_host_function(name_.c_str(), arg_size_, ret_size_);
        if (!handle_) {
            throw std::runtime_error("failed to link host function: " + name_);
        }
    }

    ~PyHostFunctionCaller() {
        if (handle_) {
            unlink_host_function(handle_);
            handle_ = nullptr;
        }
    }

    PyHostFunctionCaller(const PyHostFunctionCaller &) = delete;
    PyHostFunctionCaller &operator=(const PyHostFunctionCaller &) = delete;

    void call(py::bytes payload) {
        const std::string data = payload;
        if (data.size() > arg_size_) {
            throw std::runtime_error("argument payload exceeds arg_size");
        }

        std::fill(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(arg_size_), 0);
        std::memcpy(buffer_.data(), data.data(), data.size());

        const int ret = call_host_function(handle_, static_cast<const void *>(buffer_.data()));
        if (ret != 0) {
            throw std::runtime_error("failed to call host function");
        }
    }

    py::bytes get_response() {
        const int ret = get_response_host_function(handle_, static_cast<void *>(buffer_.data()));
        if (ret != 0) {
            throw std::runtime_error("failed to get host function response");
        }
        return py::bytes(buffer_.data(), static_cast<ssize_t>(ret_size_));
    }

    size_t arg_size() const {
        return arg_size_;
    }

    size_t ret_size() const {
        return ret_size_;
    }

private:
    std::string name_;
    size_t arg_size_;
    size_t ret_size_;
    host_function_caller handle_ = nullptr;
    std::vector<char> buffer_;
};

struct CallbackBinding {
    py::function callback;
    size_t arg_size = 0;
    size_t ret_size = 0;
    std::vector<char> response;
};

static void *dispatcher_callback(const void *args, void *user_data) {
    auto *binding = static_cast<CallbackBinding *>(user_data);
    if (!binding) {
        return nullptr;
    }

    try {
        py::gil_scoped_acquire gil;
        py::bytes request(static_cast<const char *>(args), static_cast<ssize_t>(binding->arg_size));
        py::object ret = binding->callback(request);

        if (ret.is_none() || binding->ret_size == 0) {
            return nullptr;
        }

        const std::string payload = ret.cast<std::string>();
        if (payload.size() > binding->ret_size) {
            throw std::runtime_error("callback returned payload larger than ret_size");
        }

        binding->response.assign(binding->ret_size, 0);
        std::memcpy(binding->response.data(), payload.data(), payload.size());
        return static_cast<void *>(binding->response.data());
    } catch (const py::error_already_set &err) {
        std::fprintf(stderr, "robot_ipc callback Python error: %s\n", err.what());
    } catch (const std::exception &err) {
        std::fprintf(stderr, "robot_ipc callback error: %s\n", err.what());
    }

    return nullptr;
}

class PyHostFunctionDispatcher {
public:
    explicit PyHostFunctionDispatcher(size_t max_func_count)
        : max_func_count_(max_func_count) {
        handle_ = create_host_function_dispatcher(max_func_count_);
        if (!handle_) {
            throw std::runtime_error("failed to create host function dispatcher");
        }
    }

    ~PyHostFunctionDispatcher() {
        if (handle_) {
            delete_host_function_dispatcher(handle_);
            handle_ = nullptr;
        }
    }

    PyHostFunctionDispatcher(const PyHostFunctionDispatcher &) = delete;
    PyHostFunctionDispatcher &operator=(const PyHostFunctionDispatcher &) = delete;

    void attach(
        const std::string &name,
        py::function callback,
        size_t arg_size,
        size_t ret_size
    ) {
        auto binding = std::make_unique<CallbackBinding>();
        binding->callback = std::move(callback);
        binding->arg_size = arg_size;
        binding->ret_size = ret_size;
        binding->response.assign(ret_size, 0);

        const int ret = attach_host_function_ex(
            handle_,
            name.c_str(),
            dispatcher_callback,
            static_cast<void *>(binding.get()),
            arg_size,
            ret_size
        );
        if (ret != 0) {
            throw std::runtime_error("failed to attach host function: " + name);
        }

        bindings_.push_back(std::move(binding));
    }

    void start() {
        const int ret = start_host_function_dispatcher(handle_);
        if (ret != 0) {
            throw std::runtime_error("failed to start host function dispatcher");
        }
    }

private:
    size_t max_func_count_;
    host_function_dispatcher handle_ = nullptr;
    std::vector<std::unique_ptr<CallbackBinding>> bindings_;
};

}  // namespace

PYBIND11_MODULE(_core, m) {
    m.doc() = "pybind11 core bindings for robot_ipc";

    py::class_<PyHostVariable>(m, "_HostVariable")
        .def(py::init<const std::string &, size_t>(), py::arg("name"), py::arg("size"))
        .def("write", &PyHostVariable::write, py::arg("payload"), py::arg("op_size") = 0)
        .def("read", &PyHostVariable::read, py::arg("op_size") = 0)
        .def_property_readonly("size", &PyHostVariable::size);

    py::class_<PyHostFunctionCaller>(m, "_HostFunctionCaller")
        .def(py::init<const std::string &, size_t, size_t>(), py::arg("name"), py::arg("arg_size"), py::arg("ret_size"))
        .def("call", &PyHostFunctionCaller::call, py::arg("payload"))
        .def("get_response", &PyHostFunctionCaller::get_response)
        .def_property_readonly("arg_size", &PyHostFunctionCaller::arg_size)
        .def_property_readonly("ret_size", &PyHostFunctionCaller::ret_size);

    py::class_<PyHostFunctionDispatcher>(m, "_HostFunctionDispatcher")
        .def(py::init<size_t>(), py::arg("max_func_count") = 16)
        .def("attach", &PyHostFunctionDispatcher::attach, py::arg("name"), py::arg("callback"), py::arg("arg_size"), py::arg("ret_size"))
        .def("start", &PyHostFunctionDispatcher::start);
}
