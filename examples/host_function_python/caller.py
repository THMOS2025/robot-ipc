# caller.py

import time
import datetime
from robot_ipc.robot_ipc import HostFunctionCaller

a = HostFunctionCaller("a")
while(True):
    a(datetime.datetime.now())
    print(f"call lantency = {a.get_response()}")
    time.sleep(1)

