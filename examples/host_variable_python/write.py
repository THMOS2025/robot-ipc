# write.py
#   This is an example of how to use the robot_ipc python interface. 
#   Notice that python's varible are isolated from C except using ctypes 
#   to guarantee the memory format.

from robot_ipc.robot_ipc import HostVariable
import datetime

a = HostVariable("host_variable_py")

while True:
    a.data = datetime.datetime.now()
