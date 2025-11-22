# robot_ipc.py
#   Just copy it and import

# Load dependencies first
import os
import sys
import ctypes
import pickle

_module_dir = os.path.dirname(os.path.abspath(__file__))

if os.name == 'posix':
    if os.uname().sysname == 'Darwin': # macOS
        __lib_file = os.path.join(_module_dir, 'librobot_ipc.dylib')
    else: # Linux
        __lib_file = os.path.join(_module_dir, 'librobot_ipc.so')
else: # Windows
    __lib_file = os.path.join(_module_dir, 'librobot_ipc.dll')


try:
    robot_ipc_lib = ctypes.CDLL(__lib_file)
    
    # define host_variable apis
    robot_ipc_lib.link_host_variable.argtypes = \
            [ctypes.c_char_p, ctypes.c_size_t]
    robot_ipc_lib.link_host_variable.restype = ctypes.c_void_p
    robot_ipc_lib.read_host_variable.argtypes = \
            [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t]
    robot_ipc_lib.read_host_variable.restype = ctypes.c_int
    robot_ipc_lib.write_host_variable.argtypes = \
            [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_size_t]
    robot_ipc_lib.write_host_variable.restype = ctypes.c_int
    robot_ipc_lib.unlink_host_variable.argtypes = \
            [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t]
    robot_ipc_lib.unlink_host_variable.restype = None

    # define host_function_caller apis
    robot_ipc_lib.link_host_function.argtypes = \
            [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_size_t]
    robot_ipc_lib.link_host_function.restypes = ctypes.c_void_p
    robot_ipc_lib.unlink_host_function.argtypes = [ctypes.c_void_p]
    robot_ipc_lib.unlink_host_function.restypes = ctypes.c_int
    robot_ipc_lib.call_host_function.argtypes = \
            [ctypes.c_void_p, ctypes.c_void_p]
    robot_ipc_lib.call_host_function.restypes = ctypes.c_int
    robot_ipc_lib.get_response_host_function.argtypes = \
            [ctypes.c_void_p, ctypes.c_void_p]
    robot_ipc_lib.get_response_host_function.restypes = ctypes.c_int

    # define host_function_receiver apis
    callback_func_type = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_void_p)
    robot_ipc_lib.create_host_function_dispatcher.argtypes = [ctypes.c_size_t]
    robot_ipc_lib.create_host_function_dispatcher.restypes = ctypes.c_void_p
    robot_ipc_lib.delete_host_function_dispatcher.argtypes = [ctypes.c_void_p]
    robot_ipc_lib.delete_host_function_dispatcher.restypes = ctypes.c_int
    robot_ipc_lib.attach_host_function.argtypes = \
            [ctypes.c_void_p, ctypes.c_char_p, callback_func_type, \
            ctypes.c_size_t, ctypes.c_size_t]
    robot_ipc_lib.attach_host_function.restypes = ctypes.c_int
    robot_ipc_lib.start_host_function_dispatcher.argtypes = [ctypes.c_void_p]
    robot_ipc_lib.start_host_function_dispatcher.restypes = ctypes.c_int

except Exception as e:
    print(f"Can not load {__lib_file}: \n{e}")


class HostVariable:
    class _HostVariable: # a virtual class to imply __get__ and __set__
        def __get__(self, obj, objtype):
            return obj.read()
        def __set__(self, obj, value):
            obj.write(value)

    data = _HostVariable()

    def __init__(self, name: str, max_size : int = 4096, data_format = None):
        if data_format is not None:
            max_size = ctypes.sizeof(data_format)
        self.name, self.max_size = name, max_size
        self.__p = robot_ipc_lib.link_host_variable(name.encode(), max_size)
        self.__buffer = ctypes.create_string_buffer(max_size)
        self.data_format = data_format
        if not self.__p:
            raise Exception(f"Can not link to {name}")


    def __del__(self):
        if robot_ipc_lib.unlink_host_variable(self.__p, \
                self.name.encode(), self.max_size):
            raise Exception(f"Can not unlink variable {self.__p}")


    def write(self, data, data_len : int | None = None):
        if self.data_format is None:
            data = pickle.dumps(data)
            data_len = len(data)
            if data_len > self.max_size:
                raise Exception(f"Data length {data_len} exceed max size")
            void_ptr = ctypes.c_char_p(data) # this avoid memcoping
        else:
            void_ptr = ctypes.byref(data)
            if data_len is None:
                data_len = self.max_size
        robot_ipc_lib.write_host_variable(self.__p, \
                ctypes.cast(void_ptr, ctypes.c_void_p), \
                self.max_size, \
                data_len)

    def read(self, data_len : int | None = None):
        if data_len is None:
            data_len = self.max_size
        robot_ipc_lib.read_host_variable(self.__p, \
                self.__buffer, self.max_size, self.max_size)
        if self.data_format is not None:
            return self.data_format.from_buffer(self.__buffer)
        try:
            return pickle.loads(self.__buffer)
        except Exception as e:
            return None

class HostFunctionCaller:
    def __init__(self, name, max_arg_size=4096, max_ret_size=4096):
        self.max_arg_size = max_arg_size
        self.max_ret_size = max_ret_size
        self.__buffer = (ctypes.c_char * max(max_arg_size, max_ret_size))()
        self.__p = robot_ipc_lib.link_host_function(\
                name.encode(), max_arg_size, max_ret_size)
        if not self.__p:
            raise Exception(f"Can not link to function {name}")
    def __del__(self):
        if robot_ipc_lib.unlink_host_function(self.__p):
            raise Exception(f"Can not unlink functino {self.__p}")
    def __call__(self, *args, **kwargs):
        args_data = pickle.dumps(args)
        args_len = len(args_data)
        kwargs_data = pickle.dumps(kwargs)
        kwargs_len = len(kwargs_data)
        size_field_bytes = 8
        total_size = size_field_bytes + args_len + kwargs_len

        if total_size > self.max_arg_size:
            raise Exception(f"Require max_arg_size > {total_size}")

        offset = 0
        ctypes.memmove(ctypes.addressof(self.__buffer) + offset, \
                args_len.to_bytes(size_field_bytes, sys.byteorder), \
                size_field_bytes)

        offset += size_field_bytes
        ctypes.memmove(ctypes.addressof(self.__buffer) + offset, args_data, args_len)

        offset += args_len
        ctypes.memmove(ctypes.addressof(self.__buffer) + offset, \
                kwargs_data, kwargs_len)

        robot_ipc_lib.call_host_function(self.__p, self.__buffer)

    def get_response(self):
        robot_ipc_lib.get_response_host_function(self.__p, self.__buffer)
        return pickle.loads(self.__buffer)


class HostFunctionDispatcher:
    def __init__(self, max_func_count=16):
        self.__p = robot_ipc_lib.create_host_function_dispatcher(max_func_count)
        self.__reference = []   # have to store the reference to preserve
                                # the function from gabage collation
        self.__dumps_ret = None
        if not self.__p:
            raise Exception("Can not create host function dispatcher")
    def __del__(self):
        if robot_ipc_lib.delete_host_function_dispatcher(self.__p):
            raise Exception("Can not destroy host function dispatcher")
    def attach(self, name, foo, max_sz_args = 4096, max_sz_ret = 4096):
        def __callback(data_buffer):
            buffer_wrapper = (ctypes.c_char * max_sz_args).\
                    from_address(ctypes.cast(data_buffer, ctypes.c_void_p).value)
            mv = memoryview(buffer_wrapper)

            # get the first 8 bytes as a size
            size_t_bytes = mv[0:8]
            args_size = int.from_bytes(size_t_bytes, sys.byteorder)

            # get the size of args
            args_start = ctypes.sizeof(ctypes.c_size_t)
            args_end = args_start + args_size

            # get the memory view
            args_mv = mv[args_start:args_end]
            kwargs_mv = mv[args_end:]

            # unpickle them
            unpickled_args = pickle.loads(args_mv)
            unpickled_kwargs = pickle.loads(kwargs_mv)

            # exactly call the function
            ret = foo(*unpickled_args, **unpickled_kwargs)
            if ret is None:
                return None

            # handling the return
            ret_pickled = pickle.dumps(ret)
            ret_len = len(ret_pickled)
            if ret_len > max_sz_ret:
                raise Exception(f"Return length exceed max_sz_ret")
            ret_ptr = ctypes.c_char_p(ret_pickled)
            self.__dumps_ret = ret_ptr  # Keeps the buffer alive
            return ctypes.cast(ret_ptr, ctypes.c_void_p).value
            
        c_func_ref = callback_func_type(__callback)
        self.__reference.append(c_func_ref)
        if robot_ipc_lib.attach_host_function(self.__p, name.encode(), \
                c_func_ref, max_sz_args, max_sz_ret):
            raise Exception(f"Can not attach function {foo}")
    def start(self):
        robot_ipc_lib.start_host_function_dispatcher(self.__p)


