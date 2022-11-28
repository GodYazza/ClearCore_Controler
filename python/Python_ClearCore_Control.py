from ctypes import WinDLL
from ctypes import c_int, c_float
import time
import deviceConnect
forward_position = 2000

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
print(" ")
print("Enter y to home")
theInputText = "nope"
theInputText = input(": ")
while(theInputText != "y"):
    theInputText = input(": ")
print("Homing to back sensor")
success = ClearCore.home_x()
assert success == True, "Error with Homing"

if (ClearCore.Global_Home_Success_Flag):
    print('Home to back sensor was successful')
else:
    print('Homing failed')

print(' ')
print(' ')
print('*** Program Step 3: Run Demo ***')
print(' ')

print(" ")
print("Enter y to start demo")
theInputText = "nope"
theInputText = input(": ")
while(theInputText != "y"):
    theInputText = input(": ")
success = ClearCore.ping_ClearCore()
assert success == True, "Error with getting a successful ping to and from ClearCore"
success = ClearCore.move_x_forward(forward_position)
assert success == True, "Error with moving the the forward sensor"

print(" ")
print("Enter y to move to home")
theInputText = "nope"
theInputText = input(": ")
while(theInputText != "y"):
    theInputText = input(": ")
success = ClearCore.ping_ClearCore()
assert success == True, "Error with getting a successful ping to and from ClearCore"
success = ClearCore.move_x_home()
assert success == True, "Error with moving to home position"
    

 
print(' ')
print(' ')
print('*** Program Complete ***')
print(' ')
