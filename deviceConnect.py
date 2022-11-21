import pyvisa as visa
import serial
import time

class deviceConnection():
    def connect(self, address):
        self.rm = visa.ResourceManager()
        self.comms = self.rm.open_resource(address)
        print(self.comms.query('*IDN?'))

    def printDevices(self):
        resource_list = self.rm.list_resources()
        print(resource_list)

class serialDeviceConnection():
    def connect(self, theCOMPort, baudRate):
        self.comms = serial.Serial(theCOMPort, baudRate)
    
    def disconnect(self, theCOMPort):
        self.comms.close()

    def printDevices(self):
        print("serialDeviceConnection.printDevices() is Not implemented")
        
        
#TEKNIK ClearCore Controller is COM6 connected to the right hand isde USB of my ASUS Laptop        
ClearCore_COMPORT = "COM7"
ClearCore_baudRate = 9600
class ClearCore_controller(serialDeviceConnection):
    
    def __init__(self):
        self.Global_Home_Sucess_Flag = False
        
        self.Global_Position = 0
        
        serialDeviceConnection.connet(self, ClearCore_COMPORT, ClearCore_baudRate)
        # Send a ping to the ClearCore Controller
        ping_success = -1
        ClearCore_connection_attempts = 1
        while ping_success == -1:
            self.comms.write(b"p")
            echo_input = self.comms.readline()
            #time.sleep(1)
            echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
            echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
            if echo_input_str_strip == 'CLEARCORE: Ping detected... Echo back':
                print('Successful serial connection to ClearCore via ' + ClearCore_COMPORT)
                print('Number of Ping attempts: ' + str(ClearCore_connection_attempts))
                print('\n')
                ping_success = 1
            ClearCore_connection_attempts += 1
            
    
def confrim_command(self):
        print('Asking ClearCore what it\'s last instruction was.')
        print('ClearCore responds:')
        echo_input = self.comms.readline() #Wait for ClearCore to confirm command
        echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
        echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
        print(echo_input_str_strip)
        print('\n\n\n')    
        
    
def __del__(self):
        serialDeviceConnection.disconnect(self,ClearCore_COMPORT)
        print('LCR Relay Board disconnected, presumably')

def home(self):
    home_success_flag = False
    # This function should only be called once ClearCore is ready to receive a command
        
    print('Sending command to ClearCore that we want it to Home the Y axis\n')
    self.comms.write(b"HomeY#") # Send command to ClearCore to Home Y axis
    # Now we see if the command was successfully received
    print('Confriming command by asking ClearCore what it\'s last command was.')
    print('ClearCore responds...')
    time.sleep(1)
    echo_input = self.comms.readline() #Wait for ClearCore to confirm command
    echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
    echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
    print(echo_input_str_strip)
    print(' ')
        
    print('Waiting for ClearCore to Home Y axis mover')
    time.sleep(1)
    while (not home_success_flag):
        echo_input = self.comms.readline() #Wait for ClearCore to confrim 'Home Y Axis has completed'
        echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
        echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
        if echo_input_str_strip == 'CLEARCORETrue: Home Y Axis has completed':
            print('ClearCore responds...')
            print(echo_input_str_strip)
            print(' ')
            home_success_flag = True
                
        if home_success_flag:
            self.Global_Home_Sucess_Flag = True
            return True
        else:
            self.Global_Home_Sucess_Flag = False
            return False   
        
def move(self):
        move_success_flag = False
        # This function should only be called once ClearCore is ready to receive a command and has been homed
        assert self.Global_Home_Sucess_Flag == True, "Error, can't move Y axis without homing first" 
        print('Sending command to ClearCore that we want it to move the Y axis\n')
        self.comms.write(b"MoveY#") # Send command to ClearCore to Home Y axis
        # Now we see if the command was successfully received
        print('Confriming command by asking ClearCore what it\'s last command was.')
        print('ClearCore responds...')
        # Should respond with "MoveY#"
        time.sleep(1)
        echo_input = self.comms.readline() #Wait for ClearCore to confirm command
        echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
        echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
        print(echo_input_str_strip)
        print(' ')
        
        # Requesting the movelength in mm
        print('Waiting for ClearCore to request the move length from the user...')
        print('ClearCore responds...')
        # Should respond with "Requesting Y move length in mm"
        time.sleep(1)
        echo_input = self.comms.readline() #Wait for ClearCore to confirm command
        echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
        echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
        print(echo_input_str_strip)
        print(' ')

        # Now we acutally get the user to input the Y move
        value = 70
                    
        # We should now have a valid integer that is within range
        # We can now ask ClearCore to execute the move
        print(' ')
        print('Sending command to ClearCore containing postion of Y axis mover\n')
        command_str = str(value) + "#"
        self.comms.write(bytes(command_str, 'utf-8')) 
        print('Waiting for ClearCore to execute move and respond with updated Y position...')
        time.sleep(1)
        echo_input = self.comms.readline() #Wait for ClearCore to confirm command
        echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
        echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
        print(echo_input_str_strip)
        print(' ')
        #Update Global_Y_Position in python control
        self.Global_Position = value
        print('Python Control confirms the position of the Y axis mover in mm is currently: ' + str(self.Global_Y_Position))
        print(' ')
        time.sleep(1)
        return True