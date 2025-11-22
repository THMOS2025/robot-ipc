# write.py
#   This is an example of how to use the robot_ipc python interface. 
#   Notice that python's varible are isolated from C except using ctypes 
#   to guarantee the memory format.

from robot_ipc.robot_ipc import HostVariable
import time
import numpy as np




if __name__ == "__main__":
    a = HostVariable("host_variable_py", max_size=10000000)
    data = np.random.rand(1000, 1000)


    while True:
        a.data = [time.monotonic(), data]