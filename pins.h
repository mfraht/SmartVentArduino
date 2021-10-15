#pragma once
#define CodeVersion_V0_02_Sep2018		// Arduino IC
//#define CodeVersion_V0_01_Feb2019		// Arduino module  SMD

#ifdef CodeVersion_V0_02_Sep2018	// Arduino IC

	#define CodeVersion F("V0.02_Sep2018")	// Arduino IC
	#define InitialScheduleSaveCode 180	//Change it to enforce new ROM write
	#define ClockPowerPin 6  // 11 for old board, 6 for the new one
	#define BLTPower 12    // Remove for version without BLE power
	#define BLTinterruptPin 3	// Interrupt on Arduino when data come from BLE
	#define BLTstatusPin 3	// To read the BLE status
	#define resetPin 5		// Arduino reset pin
	#define LED 4	//12 External connected LED
	#define motorPin1 7 // 6 Motor Pin numbers
	#define motorPin2 8 // 7 Motor Pin numbers
	#define motorPin3 9 // 8 Motor Pin numbers
	#define motorPin4 10  // 9 Motor Pin numbers
	#define motorStandBy 11 // Low will disable the motor
	#define TEMPERATUREPin  17  // A3 Temperature Sensor wire Pin
	#define VOLTAGE_MEASURE_PIN A0  // Voltage measuring Pin

#elif defined CodeVersion_V0_01_Feb2019		// Arduino module SMD

	#define CodeVersion F("V0.01_Feb2019")	// Arduino module
	#define InitialScheduleSaveCode 180	//Change it to enforce new ROM write
	#define ClockPowerPin 7  //11 for old board, 6 for the new one
	//#define BLTPower 12    // Remove for version without BLE power
	#define BLTinterruptPin 2	// Interrupt on Arduino when data come from BLE, 3 for the old boxes
	#define BLTstatusPin 2	// To read the BLE status, 3 for the old boxes
	#define resetPin 4		// Arduino reset pin
	#define LED 5		//12 External connected LED
	#define motorPin1 8 // 6 Motor Pin numbers
	#define motorPin2 9 // 7 Motor Pin numbers
	#define motorPin3 10 // 8 Motor Pin numbers
	#define motorPin4 11  // 9 Motor Pin numbers
	#define motorStandBy 12 // Low will disable the motor
	#define TEMPERATUREPin  3  // Temperature Sensor wire Pin, 14 for the old boxes
	#define VOLTAGE_MEASURE_PIN A3  // Voltage measuring Pin
#endif
