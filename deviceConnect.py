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
