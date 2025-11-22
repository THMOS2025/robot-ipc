# write.py
#   This is an example of how to use the robot_ipc python interface with C

from robot_ipc.robot_ipc import HostVariable
import ctypes
import time

class DataFormat(ctypes.Structure):
    # This keeps the native endianness of the machine
    
    # Set _pack_ = 1 to disable alignment padding and pack members tightly
    _pack_ = 1
    
    _fields_ = [
        ("x", ctypes.c_int),
        ("y", ctypes.c_char * 10),
        ("appendix" , ctypes.c_char * 32) # just a placeholder for unfixed length structure
    ]

print("[*] connect to variable host_variable")
a = HostVariable("host_variable_struct", data_format = DataFormat)

req_data = DataFormat()
req_data.x = 200
req_data.y = b"python"
req_data.appendix = b"pypy"

a.data = req_data

