from ctypes import WinDLL
from ctypes import c_int, c_float
import time
import deviceConnect


print(' ')
print(' ')
print('*** Program Step 1: Open Communication Ports ***')
print(' ')
ClearCore = deviceConnect.ClearCore_controller()

# Now we Home the X axis, don't forget we need to Ping the ClearCore controller first
success = ClearCore.ping_ClearCore()
assert success == True, "Error with getting a successful ping to and from ClearCore"

success = ClearCore.home()
assert success == True, "Error with Homing"

