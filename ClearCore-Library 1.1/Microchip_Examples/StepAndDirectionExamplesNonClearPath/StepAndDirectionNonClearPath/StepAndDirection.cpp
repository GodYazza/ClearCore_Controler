/*
Title: Star_Destroyer_XYZ_Mover_Code_Mk1
Author: Patrick Lawton
Version: Mk1.0
Last Edit: 06/12/2022

The purpose of this code is to operate the servo motor drive in the of the test rig located in the 
Power Electronics Labs in the Newmarket campus 

This program is not run directly by the user, but is instead loaded onto the ClearCore controller and indirectly
operated by the file: StarDestroyerCode\64-Bit\Automated_IPT_Python_Control.py

The code was built using an example project provided by TEKNIC which demonstrated the code required
to command a non-TEKNIC servo motor driver (like those on the X and Y axis movers) using the TEKNIC 
ClearCore controller.

Links to original example code (Microchip_Examples\StepAndDirectionExamplesNonClearPath\StepAndDirectionNonClearPath):
ClearCore Documentation: https://teknic-inc.github.io/ClearCore-library/
ClearCore Manual: https://www.teknic.com/files/downloads/clearcore_user_manual.pdf

General notes:
-While the TEKNIC motors are actually servo motors, they are operated in a manner almost identical to a 
 generic stepper motor
-The terms "count" or "pulse" refer to the smallest discrete movement a motor can make, i.e. a single pulse on
 the STEP terminal will cause the motor to rotate "x" degrees
-For code readability, all variables are simply declared global (memory is not an issue for such a small program)
-User functions are just located after the main() funciton, can move to a new file for readability on next version
*/



#include "ClearCore.h"
#include "User_Include_File.h"
#include <string.h>

/************** INITIALIZATION VARIABLES **************/
// Specifies which motor to move.
// Options are: ConnectorM0
#define motor_M0_X ConnectorM0 
// Select the baud rate to match the target serial device
#define baudRate 9600
// Specify which serial to use: ConnectorUsb
#define SerialPort ConnectorUsb
#define InputPort ConnectorUsb

#define PYTHON_COMMAND_SIZE_PLUS_1 16

// Global Variables
/************** GLOBAL VARIABLES **************/
int32_t global_counts_per_mm = 73;
int32_t global_X_axis_counts_per_mm = 73;  // 73 counts per motor revolution

volatile uint8_t Homing_Flag_M0_X = 0;
volatile uint8_t Moving_Forward_Flag_M0_X = 0;
volatile uint8_t Moving_Backward_Flag_M0_X = 0;
volatile uint8_t Forward_Sensor_Flag = 0;
volatile uint8_t Back_Sensor_Flag = 0;
volatile int32_t Home_Offset_M0_X = 60 * global_X_axis_counts_per_mm; // that means the offset is 60 mm

int16_t global_ping_char = -1;
uint8_t global_ready_to_receive_flag = 0;
uint8_t global_command_received_flag = 0;
char global_python_command[PYTHON_COMMAND_SIZE_PLUS_1] = ""; // initializes the message array with 15 zeros

/************** I/O SETUP **************/
// Specify which input pin to read from.
// IO-0 through A-12 are all available as digital inputs.
// Note: IO 0 through 5 are just used as indicator lights (built in LED on ClearCore controller)
// Note: The Neg_Limits are used as the homing sensors for each of the three motors
// Note: Only connectors DI-6 through A-12 can trigger interrupts.

#define Neg_Limit_M0_X_DI7 ConnectorDI7 //back sensor connected to DI-7
#define Pos_Limit_M0_X_DI6 ConnectorDI6 //Forward sensor connected to DI-6
#define Test_Digital_OutputIO0 ConnectorIO0
#define Test_Digital_OutputIO1 ConnectorIO1
#define Test_Digital_OutputIO2 ConnectorIO2
#define Test_Digital_OutputIO3 ConnectorIO3
#define Test_Digital_OutputIO4 ConnectorIO4
#define Test_Digital_OutputIO5 ConnectorIO5
#define Move_Backward_InputDI8 ConnectorDI8
#define Move_Forward_InputA9 ConnectorA9
#define Emergency_Stop_InputA10 ConnectorA10

#define IO0_ON Test_Digital_OutputIO0.State(true);
#define IO0_OFF Test_Digital_OutputIO0.State(false);
#define IO1_ON Test_Digital_OutputIO1.State(true);
#define IO1_OFF Test_Digital_OutputIO1.State(false);
#define IO2_ON Test_Digital_OutputIO2.State(true);
#define IO2_OFF Test_Digital_OutputIO2.State(false);
#define IO3_ON Test_Digital_OutputIO3.State(true);
#define IO3_OFF Test_Digital_OutputIO3.State(false);
#define IO4_ON Test_Digital_OutputIO4.State(true);
#define IO4_OFF Test_Digital_OutputIO4.State(false);
#define IO5_ON Test_Digital_OutputIO5.State(true);
#define IO5_OFF Test_Digital_OutputIO5.State(false);

// Note that all values are expressed in counts unless they have a mm suffix in the variable name

// Define the velocity and acceleration limits to be used for each move
int32_t Velocity_Limit_M0_X = 50000; // pulses/counts per sec, maximum tried with no VA is 20,000 counts per second
int32_t Acceleration_Limit_M0_X = 50000; // pulses/counts per sec^2

// Define the velocity and acceleration limits to be used for each move
int32_t Homing_Velocity_Limit_M0_X = 6000; // pulses/counts per sec
int32_t Homing_Acceleration_Limit_M0_X = 20000; // pulses/counts per sec^2

int32_t Emergency_Stop_Accel = 150000;

// Define the stopping acceleration to be used when a limit switch is triggered
int32_t Limit_Switch_Acceleration_M0_X = 150000; // pulses/counts per sec^2

//Define the Free Run velocity and acceleration limits to be used for each move
int32_t Free_Run_Velocity_Limit_M0_X = 15000; //15000
int32_t Free_Run_Acceleration_Limit_M0_X = 20000; //20000


/************** FUNCTION PROTOTYPES **************/
// Declares our user-defined helper function, which is used to command moves to
// the motor. The definition/implementation of this function is at the  bottom
// of this file.
void Move_Distance_M0_X(int32_t distance);


// Initialization
void Initialization_General();
void Intialization_ISR_Limit_Switches();
void Serial_Comms_Echo();
void Serial_Comms_Get_String();

// Homing functions
void Move_Home_M0_X();
void Move_to_Forward_M0_X();
void Move_to_Home_M0_X();

// Limit switch ISRs
void Homing_M0_X_DI7_ISR();
void Neg_Limit_M0_X_DI7_ISR();
void Pos_Limit_M0_X_DI6_ISR();

// Free Run ISRs
void Stop_Backward_DI8_ISR();
void Stop_Forward_A9_ISR() ;

// Emergency Stop ISR
void Emergency_Stop_A10_ISR();

// Comms
void erase_global_python_command();
void Serial_Comms_Get_Command();
void Serial_Comms_Get_Command_Move_X();
void Serial_Comms_Get_Command_Move_X_Demo();
void Serial_Comms_Get_Exit_Free_Run();
void integer_into_global_python_command(int32_t position_counts);

void LED_IO_Indicators_OFF();

int main() {

	// HOMING
	// The idea here is to get the motor to home in the negative direction until it reaches it's respective -ve limit switch
	// and moves away 2500 counts (5mm) in the positive direction (X and Y axis example).
	Initialization_General();
	Intialization_ISR_Limit_Switches();
	Serial_Comms_Echo();

	

    while (true) {
		/*
		This is the main program for the ClearCore controller. It receives commands from the Python Command script
		via the USB serial interface. Prior to the main() program the initialization functions are run where ClearCore waits for the
		Python Command script to ping (send a 'p' character) via the serial interface and confirm communications are active.
		
		The next step is to wait for Python Command to tell ClearCore to perform the homing movements on each of the 3 axis X, Y, and Z	
		*/

		// Keep polling the comms to see if Python Control wants to send a command
		global_ping_char = SerialPort.CharGet();
		if ((char)global_ping_char == 'p') { // AWAITING PING: STATE 0 (0000)
			SerialPort.SendLine("CLEARCORE: Ping detected. Ready to receive command"); // Replies to Python Control
			global_ready_to_receive_flag = 1; // Allows us to move to get a command
			global_ping_char = -1; // Resets the global_ping_char
			
		}  //END IF
		
		// Once we are ready to receive an instruction, start polling for a command line
		if (global_ready_to_receive_flag == 1) { // STATE 1: AWAITING COMMAND: 
			global_ready_to_receive_flag = -1;   // We can just reset this flag now, 'Serial_Comms_Get_Command' won't exit until 'global_command_received_flag' == 1
			IO0_ON // Turn IO-0 LED to show we awaiting a command	1000 (STATE 1)	
			Serial_Comms_Get_Command(); // global_command_received_flag = 1 if this succeeds, 
							
		}
		
		// By this point we should have a valid command, now we need to execute it
		if (global_command_received_flag == 1) { // STATE 2: EXECUTING COMMAND
			IO0_OFF 
			IO1_ON // 0100	(STATE 2)
		
			if (strcmp(global_python_command, "HomeX#") == 0) { // STATE 3: HOMING X
				IO0_ON
				IO1_ON // 1100 (STATE 3)
				global_command_received_flag = -1; // Reset the flag and erase the command as we are about to execute the command
				erase_global_python_command();
				
				// Python Control has requested a HomeX command, so we need to execute it
				Move_Home_M0_X();
				//Assuming home was successful, we now go back to the // AWAITING PING STATE
				LED_IO_Indicators_OFF(); // 0000 (STATE 0)
				
			}
			
			if (strcmp(global_python_command, "MoveXDemo#") == 0) { // STATE 4: Moving X to Specified point at speed
				IO0_OFF
				IO1_OFF 
				IO2_ON // 0010 (STATE 4)
				global_command_received_flag = -1; // Reset the flag and erase the command as we are about to execute the command
				erase_global_python_command();			
				// Python Control has requested a move in the X axis, now we need to tell it we want to know how much to move by in millimeters
				/* 
				*/
				SerialPort.SendLine("CLEARCORE: Requesting X move length in mm");
				/*
				Recall that there are 73 motor counts per mm of X axis movement.
	
			
				Python Control will always give a position in mm somewhere within these ranges.*/
				Serial_Comms_Get_Command_Move_X_Demo();
				
				
				//Assuming the move was successful, we now go back to the // AWAITING PING STATE
				LED_IO_Indicators_OFF(); // 0000 (STATE 0)
			} 
			
			if (strcmp(global_python_command, "MoveXHome#") == 0) { // STATE 5: Moving X to Home
				IO0_ON
				IO1_ON // 1010(STATE 5)
				global_command_received_flag = -1; // Reset the flag and erase the command as we are about to execute the command
				erase_global_python_command();			
				
				//Telling the Python that the ClearCore is moving to home
				SerialPort.SendLine("CLEARCORE: Moving to Home");
				
				//Calls Function that moves Rig to position 0
				Move_to_Home_M0_X();
				
				//Assuming the move was successful, we now go back to the // AWAITING PING STATE
				LED_IO_Indicators_OFF(); // 0000 (STATE 0)
			} 
			if (strcmp(global_python_command, "FreeRun#") == 0) { // STATE 5: Moving X to Home
				IO1_ON
				IO2_ON // 0110(STATE 6)
				global_command_received_flag = -1; // Reset the flag and erase the command as we are about to execute the command
				erase_global_python_command();			
				//Sends command that ClearCore has entered Free Running Mode
				SerialPort.SendLine("Free Running Mode");
				
				//Sets the Velocity and acceleration to the preset Free Run variables
				motor_M0_X.VelMax(Free_Run_Velocity_Limit_M0_X);
				motor_M0_X.AccelMax(Free_Run_Acceleration_Limit_M0_X);
				//Move_Backward_InputDI8.Mode(Connector::INPUT_DIGITAL);
				Move_Forward_InputA9.Mode(Connector::INPUT_DIGITAL);
				
				
				while(true){
					//Move_Backward_InputDI8.Mode(Connector::INPUT_DIGITAL);
					//Move_Forward_InputA9.Mode(Connector::INPUT_DIGITAL);
					//Serial_Comms_Get_Command;
					//Polls and checks if there is any user input from the python
					Serial_Comms_Get_Exit_Free_Run();
					
					//When user input is detected set the flag and break out of the for loop to end free run
					if (strcmp(global_python_command, "Exit#") == 0){
						global_command_received_flag = -1;
						erase_global_python_command();
						break;
					
					}
					
					//If the A9 button is clicked the ClearCore will initialize the ISR for that button and move the rig forwards, 
					//once the button is let go the ISR will run and stop the motor and return to the top of the while loop
					if(Move_Forward_InputA9.State()){
						Move_Forward_InputA9.InterruptHandlerSet(Stop_Forward_A9_ISR, InputManager::LOW);
						motor_M0_X.MoveVelocity(-Free_Run_Velocity_Limit_M0_X);
 						while (true) {
							// Keeps checking if the Stopping ISR was successfully completed
 							if (Moving_Forward_Flag_M0_X == 1) {
 								break;
 							}
 						} // END WHILE
		
						Moving_Forward_Flag_M0_X = 0;
					}
					
					//If the DI8 button is clicked the ClearCore will initialize the ISR for that button and move the rig backwards,
					//once the button is let go the ISR will run and stop the motor and return to the top of the while loop
					else if(Move_Backward_InputDI8.State()){
						Move_Backward_InputDI8.InterruptHandlerSet(Stop_Backward_DI8_ISR, InputManager::LOW);
						motor_M0_X.MoveVelocity(Free_Run_Velocity_Limit_M0_X);
 						while (true) {
 							//Keeps checking if the homing ISR was successfully completed
 							if (Moving_Backward_Flag_M0_X == 1) {
 								break;
 							}
 						} // END WHILE
						Moving_Backward_Flag_M0_X = 0;
					}
					
					Delay_ms(400);
				}
				SerialPort.SendLine("CLEARCORE: Free Running Complete");
				int32_t CurerentPos = motor_M0_X.PositionRefCommanded() / global_counts_per_mm;
				SerialPort.SendLine(-CurerentPos);
				//Assuming the move was successful, we now go back to the // AWAITING PING STATE
				LED_IO_Indicators_OFF(); // 0000 (STATE 0)
			} 
			if (strcmp(global_python_command, "MoveX#") == 0) { // STATE 4: Moving X to Specified point at speed
				IO0_ON
				IO1_ON 
				IO2_ON // 1110 (STATE 7)
				global_command_received_flag = -1; // Reset the flag and erase the command as we are about to execute the command
				erase_global_python_command();			
				// Python Control has requested a move in the X axis, now we need to tell it we want to know how much to move by in millimeters
				/* 
				*/
				SerialPort.SendLine("CLEARCORE: Requesting X move length in mm");
				/*
				Recall that there are 73 motor counts per mm of X axis movement.
	
			
				Python Control will always give a position in mm somewhere within these ranges.*/
				Serial_Comms_Get_Command_Move_X();
				
				
				//Assuming the move was successful, we now go back to the // AWAITING PING STATE
				LED_IO_Indicators_OFF(); // 0000 (STATE 0)
			}
			
			
			
		
			
		} //END STATE 2 IF
		
		
		
		
		//Delay_ms(1000);
		// We now reply to Python control saying we are ready to receive
		
		//SerialPort.Send("Ping detected... Echo back test\n"); // BREAK 1
		// Wait about 2 seconds to allow Python Control to verify the Echo and send it's string
		Delay_ms(1000);
	
    }
}


/*
Gets a command from the Python Control program.
 
This function should only be called once Python Control has successfully pinged ClearCore and ClearCore is ready
to receive a command.

It will not leave this function until "global_command_received_flag" = 1
*/
void Serial_Comms_Get_Command() {
	int16_t char_waiting = -1; // Container used to pickup the next char in the Python Command message
	uint8_t char_indexer = 0; 

	// While the command has not been confirmed, keep polling the serial line
	while(global_command_received_flag != 1) {
		
		char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer, if there is one
		while(char_waiting > 0) {
			global_python_command[char_indexer] = (char)char_waiting; // Populate the command array
			char_indexer++;
			
			if ((char)char_waiting == '#') { // Python Control will always terminate a command with # as the final character 
				global_command_received_flag = 1; // Need to let the program know we successfully received a command	
				break; // We can now leave this inner while loop 				
			}// END IF
			
			char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer	
		}// END CHARS_WAITING WHILE		
			
	}// END COMMAND_RECEIVED WHILE
	/* 
	Now we should have a character array with the message terminated with a #
	Let's see if we can transform it into a string and echo it back to Python Control
	*/
	SerialPort.Send("CLEARCORE: The Python Command was: ");
	SerialPort.SendLine(global_python_command);
	
}


/*
Gets a command from the Python Control program that moves the GA mover X axis.
 
This function should only be called once Python Control has successfully pinged ClearCore and ClearCore is ready
to receive a command.
*/


void Serial_Comms_Get_Command_Move_X() {
	int16_t char_waiting = -1; // Container used to pickup the next char in the Python Command message
	uint8_t char_indexer = 0; 

	// While the command has not been confirmed, keep polling the serial line
	while(global_command_received_flag != 1) {
		
		char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer, if there is one
		while(char_waiting > 0) {
			global_python_command[char_indexer] = (char)char_waiting; // Populate the command array
			char_indexer++;
			
			if ((char)char_waiting == '#') { // Python Control will always terminate a command with # as the final character 
				global_command_received_flag = 1; // Need to let the program know we successfully received a command	
				break; // We can now leave this inner while loop 				
			}// END IF
			
			char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer	
		}// END CHARS_WAITING WHILE		
			
	}// END COMMAND_RECEIVED WHILE

	/* 
	global_python_command should now contain our move value eg 912#
	Now we need to convert this to an integer
	*/
	int Python_Move_Position_mm;
	sscanf(global_python_command, "%d", &Python_Move_Position_mm);
	// Now we need the value in counts, max value 1,000 mm * 500 counts per mm = 500,000 counts
	int32_t Python_Move_Position_Counts = Python_Move_Position_mm * global_counts_per_mm;
	// Now we tell it to move
	motor_M0_X.VelMax(Free_Run_Velocity_Limit_M0_X);
	motor_M0_X.AccelMax(Free_Run_Acceleration_Limit_M0_X);
	Move_Distance_M0_X(Python_Move_Position_Counts + motor_M0_X.PositionRefCommanded());
	// Erase the global command which would still be storing the last move position sent by Python Control
	global_command_received_flag = -1; // Reset the flag and erase the command as we are about to execute the command
	erase_global_python_command();
	
	// Code to transform the integer value of the current motor position into the global array
	int32_t Current_X_Position_In_Counts = motor_M0_X.PositionRefCommanded();
	int32_t Current_X_Position_In_mm = Current_X_Position_In_Counts / global_counts_per_mm;
	integer_into_global_python_command(Current_X_Position_In_mm);
	SerialPort.Send("CLEARCORE: Confirms the position of the X axis mover in mm is currently: ");
	SerialPort.SendLine(Python_Move_Position_mm);
	
	// Erase the global command which would still be storing the last move position sent by Python Control
	global_command_received_flag = -1; // Reset the flag and erase the command as we are about to execute the command
	erase_global_python_command();
}



void Serial_Comms_Get_Command_Move_X_Demo() {
	int16_t char_waiting = -1; // Container used to pickup the next char in the Python Command message
	uint8_t char_indexer = 0; 

	// While the command has not been confirmed, keep polling the serial line
	while(global_command_received_flag != 1) {
		
		char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer, if there is one
		while(char_waiting > 0) {
			global_python_command[char_indexer] = (char)char_waiting; // Populate the command array
			char_indexer++;
			
			if ((char)char_waiting == '#') { // Python Control will always terminate a command with # as the final character 
				global_command_received_flag = 1; // Need to let the program know we successfully received a command	
				break; // We can now leave this inner while loop 				
			}// END IF
			
			char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer	
		}// END CHARS_WAITING WHILE		
			
	}// END COMMAND_RECEIVED WHILE

	/* 
	global_python_command should now contain our move value eg 912#
	Now we need to convert this to an integer
	*/
	int Python_Move_Position_mm;
	sscanf(global_python_command, "%d", &Python_Move_Position_mm);
	// Now we need the value in counts, max value 1,000 mm * 500 counts per mm = 500,000 counts
	int32_t Python_Move_Position_Counts = Python_Move_Position_mm * global_counts_per_mm;
	// Now we tell it to move
	motor_M0_X.VelMax(Velocity_Limit_M0_X);
	motor_M0_X.AccelMax(Acceleration_Limit_M0_X);
	Move_Distance_M0_X(Python_Move_Position_Counts + motor_M0_X.PositionRefCommanded());
	// Erase the global command which would still be storing the last move position sent by Python Control
	motor_M0_X.VelMax(Free_Run_Velocity_Limit_M0_X);
	motor_M0_X.AccelMax(Free_Run_Acceleration_Limit_M0_X);
	global_command_received_flag = -1; // Reset the flag and erase the command as we are about to execute the command
	erase_global_python_command();
	
	// Code to transform the integer value of the current motor position into the global array
	int32_t Current_X_Position_In_Counts = motor_M0_X.PositionRefCommanded();
	int32_t Current_X_Position_In_mm = Current_X_Position_In_Counts / global_counts_per_mm;
	integer_into_global_python_command(Current_X_Position_In_mm);
	SerialPort.Send("CLEARCORE: Confirms the position of the X axis mover in mm is currently: ");
	SerialPort.SendLine(Python_Move_Position_mm);
	
	// Erase the global command which would still be storing the last move position sent by Python Control
	global_command_received_flag = -1; // Reset the flag and erase the command as we are about to execute the command
	erase_global_python_command();
}

/*Used to check each while loop iteration in the Free Run state for user inputs*/
void Serial_Comms_Get_Exit_Free_Run() {
	int16_t char_waiting = -1; // Container used to pickup the next char in the Python Command message
	uint8_t char_indexer = 0;

	// While the command has not been confirmed, keep polling the serial line
	
	char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer, if there is one
	while(char_waiting > 0) {
		global_python_command[char_indexer] = (char)char_waiting; // Populate the command array
		char_indexer++;
		
		if ((char)char_waiting == '#') { // Python Control will always terminate a command with # as the final character
			global_command_received_flag = 1; // Need to let the program know we successfully received a command
			break; // We can now leave this inner while loop
		}// END IF
		
		char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer
	}// END CHARS_WAITING WHILE
	
}











/*
Called by the "Serial_Comms_Get_Command_Move_X/Y/Z()" functions

Designed to transform an argument integer value into an array of chars, specifically the "global_python_command[]" array


Example comments are for:

// Code to transform the integer value of the current motor position into the global array
int32_t Current_Z_Position_In_Counts = motor_M0_Z.PositionRefCommanded();
int32_t Current_Z_Position_In_mm = Current_Z_Position_In_Counts / global_Z_axis_counts_per_mm;
integer_into_global_python_command(Current_Z_Position_In_mm);
SerialPort.Send("CLEARCORE: Confirms the position of the Z axis mover in mm is currently: ");
SerialPort.SendLine(global_python_command);

Test with 400

*/
void integer_into_global_python_command(int32_t position_counts) {
	 // Count digits in number N
	 int m = (int)position_counts; //400
	 int digit = 0;
	 while (m) { // While m > 0
		 
		 // Increment number of digits
		 digit++;
		 
		 // Truncate the last
		 // digit from the number
		 m /= 10; 
		 // First run m = 40, and digit = 1
		 // second run m = 4, and digit = 2
		 // Third run, m = 0, and digit = 3
		 // Seems to checkout OK
	 }
	 // Declare duplicate char array
	 char global_python_command_duplicate[PYTHON_COMMAND_SIZE_PLUS_1] = ""; // initializes the duplicate message array with 15 zeros
	 
	 // Separating integer into digits and accommodate it to character array
	 int index = 0;
	 while (position_counts) { // While position_counts > 0
		 
		 // Separate last digit from
		 // the number and add ASCII
		 // value of character '0' is 48
		 global_python_command_duplicate[++index] = position_counts % 10 + '0'; 
		 
		 // Truncate the last
		 // digit from the number
		 position_counts /= 10;
		 // First run global_python_command_duplicate['0\' '0' '0\' '0\' '0\' '0\' '0\' '0\' '0\']
		 // Secon run global_python_command_duplicate['0\' '0' '0' '0\' '0\' '0\' '0\' '0\' '0\']
		 // Third run global_python_command_duplicate['0\' '0' '0' '4' '0\' '0\' '0\' '0\' '0\']
	 }
	 
	 // Reverse the array for result
	 // index should still be pointing [3], index = 3
	 int i;
	 for (i = 0; i < index; i++) {
		 global_python_command[i] = global_python_command_duplicate[index - i];
		 // First run global_python_command['4' '0\' '0\' '0\' '0\' '0\' '0\' '0\' '0\']
		 // Secon run global_python_command['4' '0' '0\' '0\' '0\' '0\' '0\' '0\' '0\']
		 // Third run global_python_command['4' '0' '0' '0\' '0\' '0\' '0\' '0\' '0\']
		 
		 
	 }
	 
	 // Char array truncate by null
	 global_python_command[i] = '\0';
 }
	



/*------------------------------------------------------------------------------
This is an initialization function which can be used to set the motor up in a default state

*/
void Initialization_General() {
	// Sets the input clocking rate.
	MotorMgr.MotorInputClocking(MotorManager::CLOCK_RATE_NORMAL);

	// Sets all motor connectors into step and direction mode.
	MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL,
	Connector::CPM_MODE_STEP_AND_DIR);

	// These lines may be uncommented to invert the output signals of the
	// Enable, Direction, and HLFB lines. Some motors may have input polarities
	// that are inverted from the ClearCore's polarity.
	motor_M0_X.PolarityInvertSDEnable(false);

	// Sets the maximum velocity and acceleration for each move
	motor_M0_X.VelMax(Velocity_Limit_M0_X);
	motor_M0_X.AccelMax(Acceleration_Limit_M0_X);
	
	// Sets up serial communication and waits up to 5 seconds for a port to open.
	SerialPort.Mode(Connector::USB_CDC);
	SerialPort.Speed(baudRate);
	uint32_t timeout = 5000;
	uint32_t startTime = Milliseconds();
	SerialPort.PortOpen();
	while (!SerialPort && Milliseconds() - startTime < timeout) {
		continue;
	}
	// Configure digital inputs
	Emergency_Stop_InputA10.Mode(Connector::INPUT_DIGITAL);

	
	// Configure IO 
	Test_Digital_OutputIO0.Mode(Connector::OUTPUT_DIGITAL);
	Test_Digital_OutputIO1.Mode(Connector::OUTPUT_DIGITAL);
	Test_Digital_OutputIO2.Mode(Connector::OUTPUT_DIGITAL);
	Test_Digital_OutputIO3.Mode(Connector::OUTPUT_DIGITAL);
	Test_Digital_OutputIO4.Mode(Connector::OUTPUT_DIGITAL);
	Test_Digital_OutputIO5.Mode(Connector::OUTPUT_DIGITAL);
	// Enables all motors.
	motor_M0_X.EnableRequest(true);
	//Enable Emergency Stop ISR
	Emergency_Stop_InputA10.InterruptHandlerSet(Emergency_Stop_A10_ISR, InputManager::LOW);

}


/*
Comms function used to ping the Python Control script to synchronize the controller
*/
void Serial_Comms_Echo() {
	//IO0_ON // TEST CODE
	int16_t input_char = -1;
	bool successful_connection = false;
	//while (input_char == -1) {
	while (!successful_connection) {
		
		// Keep reading the USB serial input until the Python script pings the controller.
		input_char = SerialPort.CharGet();

		// If there was a valid byte read-in, print it.
		if ((char)input_char == 'p') {
				Delay_ms(1000);
				// Echo the input character received.
				//SerialPort.Send("Ping detected... Echo back\n");
				SerialPort.SendLine("CLEARCORE: Ping detected... Echo back");
				input_char = -1;
				//SerialPort.Send((char)input_char);
				successful_connection = true;
		}  //END IF


	}// END WHILE
		

}

/*
Comms function called to get a command from Python Control
*/
void Serial_Comms_Get_String() {
	uint8_t command_received = 0; // Used to make sure we keep trying to receive a command until success
	int16_t input_char = -1; // Container to hold the input char, might be -1 if empty so it needs to be class int
	int16_t char_waiting = -1; // Container used to pickup the next char in the Python Command message
	uint8_t char_indexer = 0; 
	char Python_Command[PYTHON_COMMAND_SIZE_PLUS_1] = ""; // initializes the message array with 10 zeros
	/*
	First we need to get a signal from the Python Control script to indicate a sequence
	of characters are ready to be sent to the ClearCore controller
	*/
	while(command_received != 1) { // Keep trying to get a command
	// Keep reading the USB serial input until the Python script pings the controller.
		input_char = SerialPort.CharGet();
		if ((char)input_char == 'p') { // Python Control is requesting to send a command
			Delay_ms(1000);
			// We now reply to Python control saying we are ready to receive
			SerialPort.SendLine("Ping detected... Echo back test"); // BREAK 1
			// Wait about 2 seconds to allow Python Control to verify the Echo and send it's string
			Delay_ms(1000);
			
			/*
			Python Control should have sent the string, now we need to get it char-by-char 
			from the serial buffers in ClearCore
			
			PROBLEM
			The program gets to here and crashes. The ClearCore Controller lights all go off and it seems to freeze
			The problem is in the code below, probably assigning the wrong type to something and writing to memory somewhere it's not supposed to
			* Fixed chars_waiting to be int32_t
			I think the issue is using booleans. The compiler warning level must have been set low for this project as it won't
			stop you incorrectly assigning a boolean to a uint8_t which seems to subsequently cause the controller to crash
			*/
			while(command_received != 1) {

				// OK to here
				char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer
				// OK to here
				while(char_waiting > 0) {
					Python_Command[char_indexer] = (char)char_waiting;
					char_indexer++;
					// OK to here

					if ((char)char_waiting == '#') { // Python Control will always terminate a command with * as the final character 
						// Never reaches this point
						command_received = 1; // Need to let the program know we successfully received a command	
						break;					
					}// END IF
					char_waiting = SerialPort.CharGet(); // Attempts to get the next char in the buffer	
				}// END CHARS_WAITING WHILE			
			}// END INNER COMMAND RECEIVED WHILE
			
		}// END OUTER PING IF	
	}// END OUTER COMMAND RECEIVED WHILE
	// OK to here
	//SerialPort.SendLine("BREAK3");
	/* 
	Now we should have a character array with the message terminated with a *
	Let's see if we can transform it into a string and echo it back to Python Control
	*/
	SerialPort.Send("CLEARCORE: The Python Command was.. ");
	SerialPort.SendLine(Python_Command);

	
}


void Move_Home_M0_X() {
	//Delay_ms(3000);
	//int16_t count = 0;
	//int16_t input_char = -1;

	// Set the maximum speed and acceleration for the homing movements
	motor_M0_X.VelMax(Homing_Velocity_Limit_M0_X);
	motor_M0_X.AccelMax(Homing_Acceleration_Limit_M0_X);
	
	// Sets the ISR for D17 to now trigger the homing ISR
	Neg_Limit_M0_X_DI7.Mode(Connector::INPUT_DIGITAL);
	Neg_Limit_M0_X_DI7.InterruptHandlerSet(Homing_M0_X_DI7_ISR, InputManager::RISING);
	
    // Tells the motor to move in the negative direction indefinitely
    motor_M0_X.MoveVelocity(Homing_Velocity_Limit_M0_X);
	
	while (true) {
		// Keeps checking if the homing ISR was successfully completed
		if (Homing_Flag_M0_X == 1) {	
			break;
		}
	} // END WHILE

	/*
	The question is, do we simply wait for the homing action to take place? or should we make the user wait and home each axis individually?
	At this early stage, we should probably make the user wait, also it is safer to do one axis at a time. Can revisit this later.
	*/
	// RESET VELOCITY AND ACCELERATION LIMITS
	// Sets the maximum velocity for each move
	motor_M0_X.VelMax(Velocity_Limit_M0_X);
	// Set the maximum acceleration for each move
	motor_M0_X.AccelMax(Acceleration_Limit_M0_X);
	
	/*
	Reset the limit switches to operate as normal, which means
	if they are triggered the program will halt and need to be reset
	
	Also tell Python Control homing was completed
	*/
	SerialPort.SendLine("CLEARCORE: Home X Axis has completed");
	Intialization_ISR_Limit_Switches();
	Delay_ms(1000);
}



/*------------------------------------------------------------------------------
 * Move_Distance_M0_X
 *
 *    Command "distance" number of step pulses away from the current position
 *    Prints the move status to the USB serial port
 *    Returns when step pulses have completed
	  Holds the program here until all the counts have been executed
 *
 * Parameters:
 *    int distance  - The distance, in step pulses, to move
 *
 * Returns: None
 // need to comment out any serial comms
 */
void Move_Distance_M0_X(int32_t distance) {
    //SerialPort.Send("Motor_0_Z Moving distance: ");
    //SerialPort.SendLine(distance);

    // Command the move of incremental distance
    motor_M0_X.Move(-distance);

    // Waits for all step pulses to output
    //SerialPort.SendLine("Moving... Waiting for the step output to finish...");
    while (!motor_M0_X.StepsComplete()) {
        continue;
    }

    //SerialPort.SendLine("Steps Complete");
}

/*------------------------------------------------------------------------------
 * Move_To_Home_M0_X
 *
 *    Commands the ClearCore to move to zero position
 *    Prints the move status to the USB serial port
 *    Returns when step pulses have completed
	  Holds the program here until all the counts have been executed
 *
 * Parameters:
 *    int distance  - The distance, in step pulses, to move
 *
 * Returns: None
 // need to comment out any serial comms
 */
void Move_to_Home_M0_X(){
	
	motor_M0_X.VelMax(Free_Run_Velocity_Limit_M0_X);
	motor_M0_X.AccelMax(Free_Run_Acceleration_Limit_M0_X);
	
	motor_M0_X.Move(0 - motor_M0_X.PositionRefCommanded());
	
	while (!motor_M0_X.StepsComplete()) {
		continue;
	}
	motor_M0_X.VelMax(Velocity_Limit_M0_X);
	// Set the maximum acceleration for each move
	motor_M0_X.AccelMax(Acceleration_Limit_M0_X);
	
	SerialPort.SendLine("CLEARCORE: Move To Home Complete");
	Delay_ms(1000);
	
}
/*------------------------------------------------------------------------------
	Intialization_ISR_Limit_Switches

	Sets up the ISR behavior for the limit switches on M0, M1, and M2 (X, Y, and Z axis)
	
	This function should be called after homing is complete as the homing functions modify
	the ISR function call to make use of the Negative limit switches as homing sensors
 
 */
void Intialization_ISR_Limit_Switches() {
	
	// Sets up the positive and negative limit switches for motor M0 (Z axis)
	Neg_Limit_M0_X_DI7.Mode(Connector::INPUT_DIGITAL);
	Neg_Limit_M0_X_DI7.InterruptHandlerSet(Neg_Limit_M0_X_DI7_ISR, InputManager::RISING);
	Pos_Limit_M0_X_DI6.Mode(Connector::INPUT_DIGITAL);
	Pos_Limit_M0_X_DI6.InterruptHandlerSet(Pos_Limit_M0_X_DI6_ISR, InputManager::RISING);

}

/*------------------------------------------------------------------------------
	Emergency Stop ISR
	
	Sets up behavior for the Emergency Stop button for when it is in a low state
 */
void Emergency_Stop_A10_ISR() {
	motor_M0_X.MoveStopDecel(Emergency_Stop_Accel);
	while(true){
		
	}
}

/*------------------------------------------------------------------------------
	Free Run ISRs
	
	Sets up the ISR behavior for the buttons connected to DI8 and A9
	
	These functions should be called after the button is in it's low state 
 */
void Stop_Backward_DI8_ISR() {
	//Decelerates the Rig, disables the ISR and set's the ISR checking flag to 1
	motor_M0_X.MoveStopDecel(Free_Run_Acceleration_Limit_M0_X);
	Move_Backward_InputDI8.InterruptEnable(false);
	Moving_Backward_Flag_M0_X = 1;
}

void Stop_Forward_A9_ISR(){
	//Decelerates the Rig, disables the ISR and set's the ISR checking flag to 1
	motor_M0_X.MoveStopDecel(Free_Run_Acceleration_Limit_M0_X);
	Move_Forward_InputA9.InterruptEnable(false);
	Moving_Forward_Flag_M0_X = 1;
}

/*------------------------------------------------------------------------------
	Homing_M0_X_DI7_ISR
	
	ISR that activates if the Negative limit switch for M0 (Z axis) is triggered during homing operation
	
	This is why the code was not working 22.02.24 1640
	Need to comment out all comm lines which were used for debugging
 */
void Homing_M0_X_DI7_ISR() {
    if (Homing_Flag_M0_X == 1){
		/*
		Do nothing, we don't want to run this homing program twice.
		We can get multiple calls due to de-bounce, this prevents it from being homed more than once.
		*/
	}
	else {
		// Tells the motor to decelerate to a stop at the rate defined by the function argument
		motor_M0_X.MoveStopDecel(Homing_Acceleration_Limit_M0_X);
		//SerialPort.SendLine("Motor_M0_Z DI6 Homing limit triggered...");
		//SerialPort.SendLine("Motor_M0_Z stopping...");
		Delay_ms(1500);
		
		//SerialPort.Send("Motor_0_Z Current Position Reference value: ");
		//SerialPort.SendLine( motor_M0_Z.PositionRefCommanded() );
		//SerialPort.SendLine(" ");
		//Delay_ms(3000);
		
		//SerialPort.SendLine( "Resetting Position Reference Value..." );
		motor_M0_X.PositionRefSet(0);
		//SerialPort.Send("Motor_0_Z Current Position Reference value: ");
		//SerialPort.SendLine( motor_M0_Z.PositionRefCommanded() );
		//SerialPort.SendLine(" ");
		//Delay_ms(3000);
		//SerialPort.Send( "Moving to home offset position +" );
		//SerialPort.SendLine(Home_Offset_M0_Z);
		
		Move_Distance_M0_X(Home_Offset_M0_X);
		Delay_ms(1500);
		//SerialPort.SendLine("Motor_0_Z Current Position Reference should now equal home_offset value...");
		//SerialPort.Send("Motor_0_Z Current Position Reference value: ");
		//SerialPort.SendLine( motor_M0_Z.PositionRefCommanded() );
		//SerialPort.SendLine(" ");
		//Delay_ms(2000);
		//SerialPort.SendLine( "Resetting Position Reference Value..." );
		//Delay_ms(2000);
		
		motor_M0_X.PositionRefSet(0);
		//SerialPort.SendLine("Motor_0_Z Current Position Reference should now be zero...");
		//SerialPort.Send("Motor_0_Z Current Position Reference value: ");
		//SerialPort.SendLine( motor_M0_Z.PositionRefCommanded() );
		//SerialPort.SendLine(" ");
		//Delay_ms(2000);
		
		//SerialPort.SendLine("Returning to Homing_Z() function...");
	}// END ELSE

	Homing_Flag_M0_X = 1;
}

/*------------------------------------------------------------------------------
	Neg_Limit_M0_X_DI7_ISR
	
	ISR that activates if the Negative limit switch for M0 (X axis) is triggered during normal operation
 */
void Neg_Limit_M0_X_DI7_ISR() {

	motor_M0_X.MoveStopDecel(Limit_Switch_Acceleration_M0_X);
	SerialPort.SendLine("Motor_0_X Negative Limit Switch Triggered...");
	SerialPort.SendLine("Motor_0_X Stopping...");
	Delay_ms(3000);
	SerialPort.Send("Motor_0_X Current Position Reference value: ");
	SerialPort.SendLine( motor_M0_X.PositionRefCommanded() );
	Delay_ms(3000);
	SerialPort.SendLine( "System will need to be re-booted as this is classed as a critical error..." );
	SerialPort.SendLine( "Reaching the negative limit switch should not be possible..." );
	while(true) {}
}



/*------------------------------------------------------------------------------
	Pos_Limit_M0_X_DI6_ISR
 
	ISR that activates if the Positive limit switch for M0 (Z axis) is triggered during normal operation
 */
void Pos_Limit_M0_X_DI6_ISR() {

	motor_M0_X.MoveStopDecel(Limit_Switch_Acceleration_M0_X);
	SerialPort.SendLine("Motor_0_X Positive Limit Switch Triggered...");
	SerialPort.SendLine("Motor_0_X Stopping...");
	Delay_ms(3000);
	SerialPort.Send("Motor_0_Z Current Position Reference value: ");
	SerialPort.SendLine( motor_M0_X.PositionRefCommanded() );
	Delay_ms(3000);
	SerialPort.SendLine( "System will need to be re-booted as this is classed as a critical error..." );
	SerialPort.SendLine( "Reaching the positive limit switch should not be possible..." );
	while(true) {}
}



void erase_global_python_command() {
	uint8_t indexer = 0;
	
	while (indexer < PYTHON_COMMAND_SIZE_PLUS_1) {
		global_python_command[indexer] = '\0';
		indexer++;
	}
	
}
//------------------------------------------------------------------------------
//Default LED setting where all LEDS are off
void LED_IO_Indicators_OFF() {
	IO0_OFF;
	IO1_OFF;
	IO2_OFF;
	IO3_OFF;
	IO4_OFF;
	IO5_OFF;	
}