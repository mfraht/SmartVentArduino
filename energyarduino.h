#pragma once

#define _DEBUG_MSGS 0 // Set to 1 for debug printing, and to 0 to stop printing
#define appPassword "SmartEnergy0550"	// 100@SmartEnergy0550#		500@4,2018/4/4,10:47:00#
#define MaxDaySchedules 5  // Max allowed schedules/day
#define maxMessageLength (7 + MaxDaySchedules * 10 + 3)
#define MaxLastMessageTime 60000L	// Max time in mSec from last message after-which you have to send the password
#define maxNameLength 40	// Maximum number of characters for the vent name
#define BLTOnTime 1			// BLE On time in sec
#define BLTOffTime 4 		// BLE Off time in sec
#define BLT_NAME "SV1.0"	// BLE default name
#define MinVoltageForMotor 4.0	// The minimum voltage that the motor should stop after which

//#define canBLETurnOff false   // If true, BLE can be powered off

#include <avr/pgmspace.h>
#include <avr/power.h>
#include <string.h>
#include <Wire.h>  // must be induced here so that Arduino library object file references work
#include "timeDate.h"

//#include <SoftwareReset.h>

///////// Stepper Motor ////////////////////////
#include <Stepper.h>
//#include "HalfStepper.h" 
// Test Line
/// Library and variables for reading Temperature //////////////////// 
#include <OneWire.h>
OneWire  ds(TEMPERATUREPin);
/////////////////////////////////////////////////////////////////

// Libraries used for power saving //////////////////////////////////////
//#include <avr/power.h>
//#include <avr/sleep.h>
//#include <Adafruit_SleepyDog.h>
#include "LowPower.h"

//Class Saved the Current Time Information
timeDate currentTime, lastUpdateTime;

cClockROM_Module CR;
DataLoggingClass DataLogging;

float temperature_low = 1000.0, temperature_high = -1000.0, current_temp = -1000.0;

/// Motor Variables /////////////////////////////////////////////////////////////////////////////
const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
#define MaxSteps 130
int stepCount = 0;  // number of steps the motor has taken
int MotorDelay = 50;
int StepDelay = 5;
int Dir = -1;
int Speed = 10; //motor speed (max 100)
				//int Steps = ; //

Stepper myStepper(stepsPerRevolution, motorPin1, motorPin2, motorPin3, motorPin4);
//HalfStepper myStepper(stepsPerRevolution, 6, 7, 8, 9);
//bool dir = LOW;
//bool SwitchStatus = false;
bool VentOpenStauts = false;
//bool MoreThan20min = false;
bool LEDStatus = LOW;
////////////////////////////////////////////////////////////////////////////////////////////////

int currentOpenPercentage = -1; // Current vent open percentage

unsigned long NewTimer;
unsigned long OldTimer = 80000L;


// Variables used for power saving //////////////////////////////////////
//int sleepMS;
//int sleepStatus = 0;             // variable to store a request for sleep
//unsigned  long lastSleep;
unsigned  long LastMsgTime = 0L;
unsigned  long LastBLTOn = 0L;


bool BLT_PR_ON = true;
bool Disconnected = false;
uint8_t BLEisSleep = 0;	// Counter Used to indicate if the Bluetooth is active or sleep
volatile boolean InterruptedOnce = false;

////////////////////////////////////////////////////////////////////////



//bool BLTNameUpdated = false;
//bool BLTnotConnected = false;
bool receiveOk = false;
bool AccessOK = false; // App must send the password before doing any action (authenticate)
//bool BeginTrack = false;
//int IsBeingSaved = 100;
//bool AlreadySave = false;
//bool ISWD = false;
//bool SavedTime = false, SaveWDSchedules = false, SaveWESchedules = false;
bool MotorVoltLow = false;
bool DisableSchedule = false;

// LED Blinking variables
unsigned int blinkMSec = 500, blinkCount = 0;
int Header;

//assume that the maximum section of the schedule per day is 4
//12am-6am,6am-12pm,12pm-6pm,6pm-12am

//for eeprom saving 
struct ScheduleSaves
{
	int StartTimeHour;
	int StartTimeMinute;
	int OpeningPercentage = 100;
};
int ScheduleSize = sizeof(ScheduleSaves);

struct ScheduleSaves* TodaySchedules = 0;

int Today_NumberOfSchedules;

char Receive[maxMessageLength];    // used to hold the received command till fully received
char firstLine[25];
int rcvIndex = 0;
//char newbltname[20];


