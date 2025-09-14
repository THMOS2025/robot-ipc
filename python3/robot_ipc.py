# robot_ipc.py
#   Just copy it and import

# Load dependencies first
import os

for module in ['ctypes', 'pickle']:
    exec(f"try:\n\timport {module}\n"
        + f"except ImportError as e:\n"
        + f"\tprint(f\'Can not load {module}!\')\n"
        + f"\tprint(e)\n"
        + f"\tprint(f\'Try ```sudo apt install python3-{module}```\')")


if os.name == 'posix':
    if os.uname().sysname == 'Darwin':  # macOS
        __lib_file = 'librobot_ipc.dylib'
    else:  # Linux
        __lib_file = 'librobot_ipc.so'
else:  # Windows
    __lib_file = 'librobot_ipc.dll'


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
    robot_ipc_lib.start_host_function.dispatcher.restypes = ctypes.c_int

except Exception as e:
    print(f"Can not load {__lib_file}: \n{e}")


class HostVariable:
    class _HostVariable: # a virtual class to imply __get__ and __set__
        def __get__(self, obj, objtype):
            return obj.read()
        def __set__(self, obj, value):
            obj.write(value)

    data = _HostVariable()

    def __init__(self, name: str, max_size : int = 4096):
        self.name, self.max_size = name, max_size
        self.__p = robot_ipc_lib.link_host_variable(name.encode(), max_size)
        self.__buffer = ctypes.create_string_buffer(max_size)
        if not self.__p:
            raise Exception(f"Can not link to {name}")


    def __del__(self):
        if robot_ipc_lib.unlink_host_variable(self.__p, \
                self.name.encode(), self.max_size):
            raise Exception(f"Can not unlink variable {self.__p}")


    def write(self, data):
        pickled_data = pickle.dumps(data)
        pickled_len = len(pickled_data)
        void_ptr = ctypes.c_char_p(pickled_data) # this avoid memcoping
        if pickled_len > self.max_size:
            raise Exception(f"Data length {len(pickled_data)} exceed max size")
        robot_ipc_lib.write_host_variable(self.__p, \
                ctypes.cast(void_ptr, ctypes.c_void_p), \
                self.max_size, \
                pickled_len)

    def read(self):
        robot_ipc_lib.read_host_variable(self.__p, \
                self.__buffer, self.max_size, self.max_size)
        try:
            data = pickle.loads(self.__buffer)
            return data
        except Exception as e:
            return None

class HostFunctionCaller:
    def __init__(self, name, max_arg_size=4096, max_ret_size=4096):
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

        offset = 0
        ctypes.memmove(ctypes.addressof(self.__buffer) + offset, \
                args_len.to_bytes(size_field_bytes, 'little'), \
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
    def __del__(self, 

