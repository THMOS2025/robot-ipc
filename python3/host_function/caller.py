# caller.py

import time
import datetime
import robot_ipc

a = robot_ipc.HostFunctionCaller("a")
while(True):
    a(datetime.datetime.now())
    print(f"call lantency = {a.get_response()}")
    time.sleep(1)

