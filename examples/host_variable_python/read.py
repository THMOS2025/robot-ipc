# read.py
#   This is an example of how to use the robot_ipc python interface. 
#   Notice that python's varible are isolated from C except using ctypes 
#   to guarantee the memory format.

from robot_ipc.robot_ipc import HostVariable
import time

a = HostVariable("host_variable_py", max_size=10000000)

while True:
    now = time.monotonic()
    dif = now - a.data[0]
    print(dif)
