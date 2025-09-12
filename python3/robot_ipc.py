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
        self.c_buffer = ctypes.create_string_buffer(max_size)
        if not self.__p:
            raise Exception(f"Can not link to {name}")


    def __del__(self):
        robot_ipc_lib.unlink_host_variable(self.__p, \
                self.name.encode(), self.max_size)


    def write(self, data):
        pickled_data = pickle.dumps(data)
        void_ptr = ctypes.c_char_p(pickled_data) # this avoid memcoping
        if len(pickled_data) > self.max_size:
            raise Exception(f"Data length {len(pickled_data)} exceed max size")
        robot_ipc_lib.write_host_variable(self.__p, \
                ctypes.cast(void_ptr, ctypes.c_void_p), \
                self.max_size, \
                len(pickled_data))

    def read(self):
        robot_ipc_lib.read_host_variable(self.__p, \
                self.c_buffer, self.max_size, self.max_size)
        try:
            data = pickle.loads(self.c_buffer)
            return data
        except Exception as e:
            return None

