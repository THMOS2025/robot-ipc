# example.py
#   This is an example of how to use the robot_ipc python interface. 
#   Notice that python's varible are isolated from C except using ctypes 
#   to guarantee the memory format.

import robot_ipc
import datetime

a = robot_ipc.HostVariable("host_variable_py")

while True:
    now = datetime.datetime.now()
    dif = now - a.data
    print(dif)
