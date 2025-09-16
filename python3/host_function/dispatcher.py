# dispatcher.py

import time
import datetime
import robot_ipc

def foo(call_time):
    return datetime.datetime.now() - call_time

d = robot_ipc.HostFunctionDispatcher()
d.start()
d.attach("a", foo)
time.sleep(10) # the daemon will exit when the object is destroy

