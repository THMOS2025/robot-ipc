# caller.py

import robot_ipc

a = robot_ipc.HostFunctionCaller("a")
a('a', (1, 2))
print(a.get_response())
