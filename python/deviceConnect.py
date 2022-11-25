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
ClearCore_COMPORT = "COM6"
ClearCore_baudRate = 9600
class ClearCore_controller(serialDeviceConnection):
    
    def __init__(self):
        self.Global_Home_Success_Flag = False
        
        self.Global_Position = 0

        
        serialDeviceConnection.connect(self, ClearCore_COMPORT, ClearCore_baudRate)
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

def home_x(self):
        home_x_success_flag = False
        # This function should only be called once ClearCore is ready to receive a command
    
        print('Sending command to ClearCore that we want it to Home the X axis\n')
        self.comms.write(b"HomeX#") # Send command to ClearCore to Home X axis
        # Now we see if the command was successfully received
        print('Confriming command by asking ClearCore what it\'s last command was.')
        print('ClearCore responds...')
        time.sleep(1)
        echo_input = self.comms.readline() #Wait for ClearCore to confirm command
        echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
        echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
        print(echo_input_str_strip)
        print(' ')
        
        print('Waiting for ClearCore to Home X axis mover')
        time.sleep(1)
        while (not home_x_success_flag):
            echo_input = self.comms.readline() #Wait for ClearCore to confrim 'Home X Axis has completed'
            echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
            echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
            if echo_input_str_strip == 'CLEARCORE: Home X Axis has completed':
                print('ClearCore responds...')
                print(echo_input_str_strip)
                print(' ')
                home_x_success_flag = True
                
            if home_x_success_flag:
                self.Global_Home_Success_Flag = True
                return True
            else:
                self.Global_Home_Success_Flag = False
                return False         
        
def move_x_forward(self, forward_position):
        move_success_flag = False
        # This function should only be called once ClearCore is ready to receive a command and has been homed
        assert self.Global_Home_Sucess_Flag == True, "Error, can't move X axis without homing first" 
        print('Sending command to ClearCore that we want it to move the X axis\n')
        self.comms.write(b"MoveXForward#") # Send command to ClearCore to Home Y axis
        # Now we see if the command was successfully received
        print('Confriming command by asking ClearCore what it\'s last command was.')
        print('ClearCore responds...')
        # Should respond with "Move#"
        time.sleep(1)
        echo_input = self.comms.readline() #Wait for ClearCore to confirm command
        echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
        echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
        print(echo_input_str_strip)
        print(' ')
        time.sleep(1)
        command_str = str(forward_position) + "#"
        self.comms.write(bytes(command_str, 'utf-8')) # Send command to ClearCore to Home X axis
        print('Waiting for ClearCore to execute move and respond with updated X position...')
        time.sleep(1)
        echo_input = self.comms.readline() #Wait for ClearCore to confirm command
        echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
        echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
        print(echo_input_str_strip)
        print(' ')
        #Update Global_X_Position in python control
        self.Global_Position = forward_position
        print('Python Control confirms the position of the X axis mover in mm is currently: ' + str(self.Global_Position))
        print(' ')
        time.sleep(1)
        return True

def move_x_home(self):
        move_success_flag = False
        # This function should only be called once ClearCore is ready to receive a command and has been homed
        assert self.Global_Home_Sucess_Flag == True, "Error, can't move X axis without homing first" 
        print('Sending command to ClearCore that we want it to move the X axis\n')
        self.comms.write(b"MoveXHome#") # Send command to ClearCore to Home Y axis
        # Now we see if the command was successfully received
        print('Confriming command by asking ClearCore what it\'s last command was.')
        print('ClearCore responds...')
        # Should respond with "Move#"
        time.sleep(1)
        echo_input = self.comms.readline() #Wait for ClearCore to confirm command
        echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
        echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
        print(echo_input_str_strip)
        print(' ')
        print('Waiting of ClearCore to move to Forward Sensor')
        while (not move_success_flag):
            echo_input = self.comms.readline() #Wait for ClearCore to confrim 'Home X Axis has completed'
            echo_input_str = echo_input.decode("utf-8") # Decodes from b'string' or bytes type, to string type 
            echo_input_str_strip = echo_input_str.rstrip('\r\n')# Removes \n and \r from the recived string
            if echo_input_str_strip == 'CLEARCORE: Move To Home Complete':
                print('ClearCore responds...')
                print(echo_input_str_strip)
                print(' ')
                move_success_flag = True
        
        time.sleep(1)
        return True