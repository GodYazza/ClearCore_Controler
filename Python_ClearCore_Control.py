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

print(' ')
print(' ')
print('*** Program Step 2 Homing to back sensor ***')
print(' ')
print("Homing to back sensor")
success = ClearCore.home()
assert success == True, "Error with Homing"

if (ClearCore.Global_Home_Sucess_Flag):
    print('Home to back sensor was successful')
else:
    print('Homing failed')

print(' ')
print(' ')
print('*** Program Step 3: Run Demo ***')
print(' ')

success = ClearCore.move_x_forward
assert success == True, "Error with moving the the forward sensor"

success = ClearCore.reset_x
assert success == True, "Error with moving to home position"
    
print(' ')
print(' ')
print('*** Program Complete ***')
print(' ')
