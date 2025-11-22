# read.py
#   This is an example of how to use the robot_ipc python interface. 
#   Notice that python's varible are isolated from C except using ctypes 
#   to guarantee the memory format.

from robot_ipc.robot_ipc import HostVariable
import time
import pickle
import numpy as np



if __name__ == "__main__":
    a = HostVariable("host_variable_py", max_size=10000000)
    data = np.random.rand(1000, 1000)


    while True:
        now = time.monotonic()
        dif = now - a.data[0]
        
        t1 = time.monotonic()
        _ = [now, data].copy()  # write once
        _ = data.copy()  # read once
        t2 = time.monotonic()
        read_time = t2 - t1
        
        t3 = time.monotonic()
        fuck = pickle.dumps(data)
        fucklen = len(fuck)
        t4 = time.monotonic()
        pickle_delay = t4 - t3
        
        print(f"IPC delay: {dif * 1000:.5f} ms, read time: {read_time * 1000:.5f} ms, pickle delay: {pickle_delay * 1000:.5f} ms")
