from ctypes import c_int, c_float
import time
import deviceConnect
forward_position = 3700 #Distance for the Rig to move forward in demo mode in MM
#Setting the limits of the rig
forward_max = 3950
backward_max = 0


print(' ')
print('*********** Main Program ***********')
print(' ')
print('Authors: Yang Qian')


theTextInput = "Nope"
while (theTextInput != ""):
    theTextInput = input("Hit enter to start: ")

print(' ')
print(' ')
print(' ')
# Test code for ClearCore controller serial USB connection 
# and establishes connection to the ClearCore
ClearCore = deviceConnect.ClearCore_controller()

#If the demo position the user wants is out of range, keep asking for demo positions untill they choose a number in range
while(forward_position < backward_max or forward_position > forward_max):
    print("The demo position is out of range")
    print("Please enter a position in range between " +str(backward_max) + "mm and " + str(forward_max) + "mm")
    forward_position = int(input(":"))

theTextInput = "nope"

#Connection has been secure, now allows user to select which 
#state they want the ClearCore to operate in depending on their inputs
while (theTextInput != "e"):
    theTextInput = "nope"
    #While the input is not a selectable state, the script will ask which state
    #the user would like the ClearCore in
    while ((theTextInput != "f") and (theTextInput != "h") and (theTextInput != "d") and (theTextInput != "m") and (theTextInput != "r") and (theTextInput != "e")):
        print(' ')
        print(' ')
        print('*********** Mode Select ***********')
        print(' ')
        print("Please enter the mode you would like")
        print('Enter "f" to enter Free Run Mode')
        print('Enter "h" to home the rig')
        print('Enter "d" to enter Demo Mode')
        print('Enter "m" to move rig to an input position')
        print('Enter "r" to move to rig to Home Position')
        print('Enter "e" to exit program')
        theTextInput = input(":")
    print(" ")

    if theTextInput == "f":
        #Enters Free Running Mode
        print("Entering Free Running Mode")
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
        if ClearCore.Global_Home_Success_Flag == False:
            print("Cannot enter Demo Mode as Rig has not been homed")
            
        elif ClearCore.Global_Position == forward_position:
            print("Cannot enter Demo Mode as Rig is already at demo position")
        else:
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
                success = ClearCore.move_x_demo(forward_position)
                assert success == True, "Error with moving the the forward sensor"
            else:
                print("Exiting Demo Mode")
    
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

    elif theTextInput == "m":
        #First checks that the rig has been homed
        move = -5
        if ClearCore.Global_Home_Success_Flag == False:
            print("Cannot enter move as Rig has not been homed")
        else:
            print("Move ClearCore to in mm between(" + str(backward_max) + "mm to " + str(forward_max) +"mm):")
            #Asks for input position and will keep asking untill valid position is given
            move = int(input(": "))
            while(move < backward_max or move > forward_max):
                print("Invalid input or position entered is out of range")
                print("Please enter a position in range between " +str(backward_max) + "mm and " + str(forward_max) + "mm")
                move = int(input(": "))
            #Checks if the ClearCore is already at the input position
            if(move == ClearCore.Global_Position):
                print("Rig is already at position " + str(move) +"mm")
            else:
                #All conditions pass and move the Rig
                success = ClearCore.ping_ClearCore()#Ping the ClearCore to allow it to accept input
                assert success == True, "Error with getting a successful ping to and from ClearCore"
                success = ClearCore.move_x(move)
                assert success == True, "Error with moving the the forward sensor"
    
    time.sleep(1)

    
            



print(' ')
print(' ')
print('*** Program Complete ***')
print(' ')