# read.py
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



if __name__ == "__main__":
    print("[*] connect to variable host_variable")
    a = HostVariable("host_variable_struct", data_format = DataFormat)

    while(True):
        # prevent from misalign: you call multiple times if you do like:
        #    print(f"[+] read: ( {a.data.x}, {a.data.y}, {a.data.appendix}, {a.data.appendix[16:]} )")
        # these four value may not be within one frame. 
        res_data = a.data
        print(f"[+] read: ( {res_data.x}, {res_data.y}, {res_data.appendix})")
        time.sleep(0.5)

