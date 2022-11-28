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
theTextInput = "nope"

while (theTextInput != "e"):
    while ((theTextInput != "f") and (theTextInput != "h") and (theTextInput != "d") and (theTextInput != "r") and (theTextInput != "e")):
        print("Please enter the mode you would like")
        print('Enter "f" to enter Free Moving Mode')
        print('Enter "h" to home the rig')
        print('Enter "d" to enter Demo Mode')
        print('Enter "h" to move to rig to Home Position')
        print('Enter "e" to escape program')
        theTextInput = input(":")
    print(" ")

    if theTextInput == "f":
        #do this
        success = ClearCore.home_x()
 
    elif theTextInput == "h":
        print('Enter "y" to home')
        print('Enter "x" to exit homing')
        theInputText = "nope"
        theInputText = input(": ")
        while(theInputText != "y" and theInputText != "x"):
            theInputText = input(": ")
        if(theInputText == "y"):
            if ClearCore.Global_Home_Success_Flag == False:
                print("Homing to back sensor")
                success = ClearCore.home_x()
                assert success == True, "Error with Homing"

                if (ClearCore.Global_Home_Success_Flag):
                    print('Home to back sensor was successful')
                else:
                    print('Homing failed')
            
            else:
                print("Rig has already been homed, cannot home again")
    
        else:  
            print("Homing skipped")
        
    elif theTextInput == "d":
        if ClearCore.Global_Home_Success_Flag == True:
            print("Entering Demo Mode")
            print('Enter "y" to start demo')
            print('Enter "x" to exit demo mode')
            theInputText = "nope"
            theInputText = input(": ")
            while(theInputText != "y" and theInputText != "x"):
                theInputText = input(": ")
            if theInputText == "y":
                success = ClearCore.ping_ClearCore()
                assert success == True, "Error with getting a successful ping to and from ClearCore"
                success = ClearCore.move_x_forward(forward_position)
                assert success == True, "Error with moving the the forward sensor"

            else:
                print("Exiting Demo Mode")


        else:
            print("Cannot enter Demo Mode as Rig has not been homed")
    
    elif theTextInput == "r":
        if ClearCore.Global_Home_Success_Flag == True:
            if ClearCore.Global_Position == 0:
                print("Entering Demo Mode")
                print('Enter "y" to reset Rig to home')
                print('Enter "x" to reset')
                theInputText = "nope"
                theInputText = input(": ")
                while(theInputText != "y" and theInputText != "x"):
                    theInputText = input(": ")
                if theInputText == "y":
                    success = ClearCore.ping_ClearCore()
                    assert success == True, "Error with getting a successful ping to and from ClearCore"
                    success = ClearCore.move_x_home()
                    assert success == True, "Error with moving the the forward sensor"

                else:
                    print("Exiting reset")
            
            else:
                print("Rig is already at home")

        else:
            print("Cannot reset to home as Rig has not been homed")


print(' ')
print(' ')
print('*** Program Complete ***')
print(' ')