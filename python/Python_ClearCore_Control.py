from ctypes import WinDLL
from ctypes import c_int, c_float
import time
import deviceConnect
forward_position = 2400 #Distance for the Rig to move forward in demo mode in MM

print(' ')
print('*********** Main Program ***********')
print(' ')
print('Authors:')


theTextInput = "Nope"
while (theTextInput != ""):
    theTextInput = input("Hit enter to start: ")

print(' ')
print(' ')
print('*** Program Step 1: Open Communication Ports ***')
print(' ')
# Test code for ClearCore controller serial USB connection 
# and establishes connection to the ClearCore
ClearCore = deviceConnect.ClearCore_controller()

theTextInput = "nope"

#Connection has been secure, now allows user to select which 
#state they want the ClearCore to operate in depending on their inputs
while (theTextInput != "e"):
    theTextInput = "nope"
    #While the input is not a selectable state, the script will ask which state
    #the user would like the ClearCore in
    while ((theTextInput != "f") and (theTextInput != "h") and (theTextInput != "d") and (theTextInput != "r") and (theTextInput != "e")):
        print("Please enter the mode you would like")
        print('Enter "f" to enter Free Moving Mode')
        print('Enter "h" to home the rig')
        print('Enter "d" to enter Demo Mode')
        print('Enter "r" to move to rig to Home Position')
        print('Enter "e" to exit program')
        theTextInput = input(":")
    print(" ")

    if theTextInput == "f":
        #do this
        success = ClearCore.ping_ClearCore()#Ping the ClearCore to allow it to accept input
        assert success == True, "Error with getting a successful ping to and from ClearCore"
        success = ClearCore.free_run()
        assert success == True, "Error with running Free Run mode"
 
    elif theTextInput == "h":
        #Confirms the user wants to home the rig
        print('Enter "y" to home')
        print('Enter "x" to exit homing')
        theInputText = "nope"
        theInputText = input(": ")
        while(theInputText != "y" and theInputText != "x"):
            theInputText = input(": ")
        if(theInputText == "y"):
            #Checks that the rig has not been homed already
            if ClearCore.Global_Home_Success_Flag == False:
                #All conditions past so rig will now home to the back sensor
                print("Homing to back sensor")
                success = ClearCore.ping_ClearCore()#Ping the ClearCore to allow it to accept input
                assert success == True, "Error with getting a successful ping to and from ClearCore"
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
        #First checks that the rig has been homed
        if ClearCore.Global_Home_Success_Flag == True:
            #Confirms that the user wants to run the demo
            print("Entering Demo Mode")
            print('Enter "y" to start demo')
            print('Enter "x" to exit demo mode')
            theInputText = "nope"
            theInputText = input(": ")
            while(theInputText != "y" and theInputText != "x"):
                theInputText = input(": ")
            if theInputText == "y":
                #Now the rig will move to te specified point that is stored in variable forward_Position
                success = ClearCore.ping_ClearCore()#Ping the ClearCore to allow it to accept input
                assert success == True, "Error with getting a successful ping to and from ClearCore"
                success = ClearCore.move_x_forward(forward_position)
                assert success == True, "Error with moving the the forward sensor"

            else:
                print("Exiting Demo Mode")


        else:
            print("Cannot enter Demo Mode as Rig has not been homed")
    
    elif theTextInput == "r":
        #Checks the the ClearCore has been homed first
        if ClearCore.Global_Home_Success_Flag != True:
            print("Cannot reset to home as Rig has not been homed")
            #Checks the position of the Rig is not already at home
        elif ClearCore.Global_Position == 0:
            print("Rig is already at home")
            #Confirms the user wants to move to home
        else:
            print("Entering Demo Mode")
            print('Enter "y" to reset Rig to home')
            print('Enter "x" to exit moving to home')
            theInputText = "nope"
            theInputText = input(": ")
            while(theInputText != "y" and theInputText != "x"):
                theInputText = input(": ")
            if theInputText == "y":
                #Now the ClearCore will move the rig to position 0 which is home
                success = ClearCore.ping_ClearCore()#Ping the ClearCore to allow it to accept input
                assert success == True, "Error with getting a successful ping to and from ClearCore"
                success = ClearCore.move_x_home()
                assert success == True, "Error with moving the the forward sensor"

            else:
                print("Exiting reset")
            



print(' ')
print(' ')
print('*** Program Complete ***')
print(' ')