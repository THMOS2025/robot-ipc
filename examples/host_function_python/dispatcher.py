# dispatcher.py

import time
import datetime
from robot_ipc.robot_ipc import HostFunctionDispatcher

def foo(call_time):
    return datetime.datetime.now() - call_time

d = HostFunctionDispatcher()
d.start()
d.attach("a", foo)
time.sleep(10) # the daemon will exit when the object is destroy

