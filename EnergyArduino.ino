
#include "pins.h"
#include "dataLogging.h"
#include <LowPower.h>
#include <uRTCLib.h>
#include "ROMmodule.h"
#include "energyarduino.h"
////////////////////////////////////////////////////////////

//#include "Time.h"
//  711@11:59|34,12:01|44,12:02|54,12:03|64#
// 700@4,2017/2/1,11:56#


void setup() {
	// disable ADC (analog to digital conversion)
	/*ADCSRA = 0;
	power_adc_disable(); // ADC converter
	power_twi_disable(); // TWI (I2C)
	*/ // 9600
	//SWRST_DISABLE(); //disable the reset 
	//delay(100);
	/*int x;
	x = 1;*/
	Serial.begin(57600);
	delay(50);
	checkBLEconfig();

	Serial.println(F("000@#")); delay(10);	// Starting up Message
	//Serial.println("********");
	//Serial.println("AT+BAUD7");	// Set the Bluetooth Baud rate to 57600
	//Serial.println("AT + ADVI1"); // Set Advertising rate to 500mSec
	//pinMode(Switch, INPUT);
	
	pinMode(BLTstatusPin, INPUT);	//	BLE
#ifdef BLTPower
	pinMode(BLTPower, OUTPUT);	// <<<<---- Remove
	delay(10);
	digitalWrite(BLTPower, BLT_PR_ON);
#endif // BLTPower
	
	pinMode(ClockPowerPin, OUTPUT);	// 
	CR.init();
	CR.syncClockFromModule(&currentTime);	// Read the clock from the module
	
	DataLogging.init(&CR, &DisableSchedule);

	pinMode(LED, OUTPUT);
	pinMode(motorStandBy, OUTPUT);
	digitalWrite(motorStandBy, LOW);	// Standby the motor
	/*
	/////// Stepper Motor //////////////////////////////////////////////
	AFMS.begin();  // create with the default frequency 1.6KHz
	//AFMS.begin(1000);  // OR with a different frequency, say 1KHz

	myMotor->setSpeed(10);  // 10 rpm
	/////////////////////////////////////////////////////////////////////
	*/

	createInitialSchedules(false);  //If no schedule is saved, it will create initial one
	printTimeDateDbg(&currentTime);
	loadTodaysSchedule();
	/*for (int i = 0; i < Today_NumberOfSchedules; i++) {
	ReadFromEEPROM(&ArrayWDSchedules[i], i, &CheckSaved, &WD_NumberOfSchedules, 1);
	}*/
	//ArrayWDSchedules = Temp_ArrayWDSchedules;
	//debugPrint(("TODAY Schedule:"), 15, true);
	//debugPrint("---------------", 10, true);
	if (_DEBUG_MSGS)
		for (int i = 0; i < Today_NumberOfSchedules; i++) {    // Display the saved WD      
			printSchedule(&TodaySchedules[i]);
		}

	/*}
	else {
	createInitialSchedules();
	AlreadySave = false;
	Serial.print(" not saved");
	}*/
	// Used to Calculate the battery Voltage)//////////////////////////////////
	analogReference(INTERNAL);
	//burn some ADC readings after reference change
	for (int i = 0; i < 8; i++) analogRead(A0);
	//////////////////////////////////////////////////////////////////////////

	//// For Watch Dog /////////////////////////////////////////////////////
	//int countdownMS = Watchdog.enable();
	////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////
	pinMode(BLTinterruptPin, INPUT);
	//attachInterrupt(BLTinterruptPin, pin_ISR, CHANGE); FALLING
	attachInterrupt(digitalPinToInterrupt(BLTinterruptPin), pin_ISR, RISING);
	LED_FLASH(5, 1000);			// Indicate that it started

	ReadTemp();
}

// Check BLE configuration. Configure it if needed
void checkBLEconfig() {
	// Check for the correct Baud-rate
	const int DELAY = 500;
	Serial.println(F("AT"));	// 
	delay(DELAY);
	readSerial('\n');
	if (strncmp(Receive, "OK", strlen("OK")) != 0) {
		Serial.println(Receive);	delay(DELAY);	// <<<<< debug
		Serial.begin(9600);
		delay(DELAY);
		Serial.println(F("Baudrate wasn't correct"));	delay(DELAY);	// <<<<< debug		
	}
	else {	// If the rate is ok, check of the name is correct. If correct then return
		Serial.println(F("AT+NAME"));	// Get the BLE name
		delay(DELAY);
		readSerial('\n');
		String n = F("+NAME=");
		n += F(BLT_NAME);		
		//if (strncmp(Receive, "+NAME=SV1.0", strlen("+NAME=SV1.0")) == 0) return;	// Return if the name is OK
		if (strncmp(Receive, n.c_str(), n.length()) == 0) {
			Serial.println(F("Name is correct#"));	delay(DELAY);	// <<<<< debug
			return;	// Return if the name is OK
		}
	}
	Serial.println(F("Program everything"));	delay(DELAY);	// <<<<< debug
	Serial.println(F("AT+TYPE0"));	delay(DELAY);	// To Set Module Bond Mode to 0 for no pin
	Serial.println(F("AT+ADVI5"));	delay(DELAY);	// TO Set the Advertising interval to 546.25 ms
	Serial.print(F("AT+NAME")); Serial.println(F(BLT_NAME)); delay(DELAY);	// To Set the name to SV1.0
	Serial.println(F("AT+BAUD7"));	delay(DELAY);	// Set the Bluetooth Baud rate to 57600
	Serial.println(F("AT+POWE3"));	delay(DELAY);	// TO Set the BLT power to maximum 3
	Serial.println(F("AT+PWRM0"));	delay(DELAY);	// TO Set the sleep mode to Auto sleep Mode
	Serial.println(F("AT+NOTI0"));	delay(DELAY);	// TO Set Notify connection to on
	Serial.println(F("AT+BAUD7"));	delay(DELAY);	// TO Set The BAUD rate To 57600
	Serial.println(F("AT+RESET"));	delay(DELAY);	// TO apply all previous settings, SW reboot

	Serial.println(F("Programmed.."));	delay(DELAY);	// <<<<< debug
	Serial.begin(57600);
	delay(DELAY);
	Serial.println(F("Working on the new config."));	delay(DELAY);	// <<<<< debug
	// Flash the LED
	LEDStatus = HIGH;
	blinkCount = 2;
	blinkMSec = 1000;
}

// Read serial till given terminator. The read will be saved in Receive[]
void readSerial(char termonator) {
	char temp = 0;
	rcvIndex = 0;
	while (Serial.available())
	{
		temp = Serial.read();
		//if (temp != 'O' || temp != 'K')
		//Serial.print(temp); 

		if (!isprint(temp)) { // Check to make sure the received characters are valid
							  //Serial.print(F("Not Printable# ")); Serial.println((int)temp);  delay(50);
			if (temp != 10 && temp != 13) rcvIndex = 0;	// Only skip LF & CR
			continue;
		}
		Receive[rcvIndex++] = temp;

		if (temp == termonator) {
			Receive[rcvIndex] = '\0';
			rcvIndex = 0;
			break;
		}
		// Wait if no data is available, max 10 mSec
		for (uint8_t i = 0; (i < 10) && !Serial.available(); i++)
			delay(1);
	}
}

void loadTodaysSchedule() {
	if (TodaySchedules) delete(TodaySchedules);
	TodaySchedules = ReadDayScheduleFromEEPROM(currentTime.Current_WeekDay, &Today_NumberOfSchedules);
}

void printSchedule(struct ScheduleSaves* s) {
	debugPrint("Current Schedule: ", 10, false);
	debugPrint(s->StartTimeHour, 10, false); debugPrint(":", 5, false);
	debugPrint(s->StartTimeMinute, 10, false); debugPrint(" -> ", 5, false);
	debugPrint(s->OpeningPercentage, 10, true);
}

void copySchedule(struct ScheduleSaves* source, struct ScheduleSaves* destination) {
	destination->OpeningPercentage = source->OpeningPercentage;
	destination->StartTimeHour = source->StartTimeHour;
	destination->StartTimeMinute = source->StartTimeMinute;
}

/**
* Name: strSplitCount
* Description:  Takes character array and returns the number of pieces in the array.
* @param char * input: the schedules character array
* @param char splitter: splitter for separating the character array
*/
int strSplitCount(char * input, char splitter) {
	//Serial.print("In the strSplitCount Function"); delay(800);
	int pieces = 0;
	for (unsigned int i = 1; i < strlen(input) - 1; i++) {
		if (input[i] == splitter) {
			pieces++;
		}
	}
	//Serial.print("Out of the strSplitCount Function"); delay(800);
	return ++pieces;
}

/**
* Name: strSplitNo
* Description:  Takes character array and returns the piece with the specified index
* @param char * input: the schedules character array
* @param char * out: the schedules character array (already been separated) Output
* @param char outSize: the size of the output character array
* @param char splitter: splitter for separating the character array
* @param int pieceIndex: the index piece to be extracted.
*/
void strSplitNo(char * input, char * out, int outSize, char splitter, int pieceIndex) {
	//Serial.print("In the strSplitNo Function"); delay(800);
	//Serial.print(input);  delay(100);
	//Serial.print(outSize);  delay(100);
	int last = -1; int piece = 0;
	piece = 0; int i;
	for (i = 1; i < (int)strlen(input) + 1; i++) {
		if (input[i] == splitter) {
			if (piece == pieceIndex) {
				break;
			}
			last = i;
			piece++;
		}
	}
	//Serial.print(piece);  delay(100);
	//*pout = (char*)malloc(i-last);
	int l = (i - last < outSize) ? i - last : outSize;
	if (input[last + 1] == splitter) {
		strncpy(out, input + last + 2, --l);
	}
	else
		strncpy(out, input + last + 1, l);
	out[l - 1] = 0;
	//Serial.print("Out of the strSplitNo Function"); delay(800);
	return;
}

/**
* Name: FindHeader
* Description: extract the Header information from the String Message
* @param char Message[250]: (Input)the original Message received form the Serial port
* return: integer value of the Header
* e.g. 620@50#
*/
int FindHeader(char *Message) {
	//Serial.print("In the FindHeader Function: "); delay(100);
	//Serial.print(Message); delay(300);
	int commaIndex = strcspn(Message, "@");
	strncpy(firstLine, Message, commaIndex);
	firstLine[commaIndex] = 0;
	Header = atoi(firstLine);

	return Header;
}


/**
* Name: BackToDefault
* Description:this function is to make the vent back to the close status
*/
void BackToDefault() {
	//if (MotorVoltLow) return;
	//debugPrint(("In the BackToDefalut Function"), 50, true);
	//digitalWrite(Switch, HIGH); // pull-up the switch pin
	//if vent does not hit the switch it will be HIGH
	//when hit the status comes to LOW  && NofSteps <= 10
	//SwitchStatus = HIGH;
	//int smallSteps = 10;
	//int NofTrials = MaxSteps/ smallSteps;
	digitalWrite(motorStandBy, HIGH);	// motor active
	myStepper.setSpeed(Speed);
	//while (SwitchStatus == HIGH && NofTrials)
	{
		myStepper.step(MaxSteps * Dir * -1); //delay(MotorDelay);
											 //TurnMotorOFF();
											 //delay(StepDelay);
											 //SwitchStatus = digitalRead(Switch); delay(50);
											 //NofTrials--;
	}
	TurnMotorOFF();

	//Watchdog.reset();
	//debugPrint(("Out of the BackToDefalut Function", 50, true);
}


/**
* Name: TurnMotorON
* Description: Turn On or the Vent based on the Opening Percentage.
* Determine what number of steps (max 10) and motor speed (max 100)
* @param double StepsPersentage: (Input)the Opening percentage of the Vent to be Opened
*/
void TurnMotorON(double StepsPersentage) {
	//if (MotorVoltLow) return;
	// Determine what number of steps (max 10) and motor speed (max 100)
	//debugPrint("In the TurnMotorON Function", 15, true);
	int Steps = MaxSteps * (StepsPersentage / 100); //calculate the steps for Moving the Motor

													//for (int i = 0; i <= Steps; i++)
	digitalWrite(motorStandBy, HIGH);	// motor active
	{
		myStepper.setSpeed(Speed);
		myStepper.step(Steps*Dir); //delay(MotorDelay);
								   //TurnMotorOFF();
								   //delay(StepDelay);

	}
	TurnMotorOFF();
	//Serial.print(F("out of the MotorRun Function"));  delay(100);
	//Watchdog.reset();
}

void TurnMotorOFF(void)
{
	digitalWrite(motorPin1, LOW);
	digitalWrite(motorPin2, LOW);
	digitalWrite(motorPin3, LOW);
	digitalWrite(motorPin4, LOW);
	digitalWrite(motorStandBy, LOW);  // Standby the motor
}

/**
* Name: OpenCloseVent
* Description: Turn On or Off the Vent based on the String Message.
* @param String Message: (Input)the original Message received form the Serial port
* e.g. 620@50#
*/
void OpenCloseVent(int NEWOpeningPercentage) {
	//Watchdog.reset();
	/*int AT = Message.indexOf('@');
	int END = Message.lastIndexOf('#');
	String S_OpeningPercentage = Message.substring(AT + 1, END);
	int NEWOpeningPercentage = S_OpeningPercentage.toInt();*/

	if (currentOpenPercentage == NEWOpeningPercentage)
	{
		//debugPrint("No new activities in the Schedule,\nLast opening Percentage is ", 10, false);
		//debugPrint(currentOpenPercentage, 10, false);
		//debugPrint(" %", 5, true);
		return;
	}

	if (fReadVcc() < MinVoltageForMotor) {	// Check for low batteries. Plash the LED and return.
		LED_FLASH(20, 5000);
		Serial.println(F("590@Please replace Batteries.#"));	delay(10);	//
		return;
	}

	//debugPrint(currentOpenPercentage, 20, true);
	//debugPrint("In the Open/Close Vent Function", 50, true);
	//need to be discussed
	if (NEWOpeningPercentage <= 10) {// 10% means Close the Vent
									 //close it directly
		// BackToDefault();
		TurnMotorON(-1 * currentOpenPercentage);
		VentOpenStauts = false;
		//debugPrint("Vent Closed", 10, true);
	}
	else {
		//BackToDefault();
		TurnMotorON((double)NEWOpeningPercentage - currentOpenPercentage);
		VentOpenStauts = true;
		//set the begin point and send a signal back to android phone
		//let the phone know when the vent begins
		//debugPrint("Vent Opened by ", 5, false);
		//debugPrint(NEWOpeningPercentage, 10, false);
		//debugPrint(" %", 5, true);
	}
	currentOpenPercentage = NEWOpeningPercentage;
	//debugPrint("Out of the Open/Close Vent Function", 15, true);
}

/**
* Name: BlinkLED
* Description: Blink the LED with 500 mSec ON and blinkMSec OFF.
*/
void BlinkLED() {
	static unsigned long oldT = NewTimer;
	if (blinkCount < 1)     return;

	//LEDStatus = HIGH;
	if (LEDStatus) {
		if ((NewTimer - oldT) < 500) return;		
	}
	else {
		if ((NewTimer - oldT) < blinkMSec) return;	
	}
	LEDStatus = !LEDStatus;
	if (LEDStatus) 	blinkCount--;	// Decrement only on High
	if (blinkCount == 0)	LEDStatus = LOW;	// Turn Off at the end of the count
	//debugPrint("LED Status: ", 15, false);
	debugPrint(LEDStatus, 5, true);
	oldT = NewTimer;
	digitalWrite(LED, LEDStatus);   // turn the LED on (HIGH is the voltage level)
	/*delay(500);                       // wait for half second
	digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
	delay(500);                       // wait for half second
	}*/

}
/**
Name: LED_FLASH
Flashes the LED for the given number of times with the given delay in mSec
*/
void LED_FLASH(int blink_count, unsigned int delayMS) {
	LEDStatus = HIGH;
	blinkCount = blink_count;
	blinkMSec = delayMS;
}
/*
* Name: ExtractCurrentTime
* Description: get the current time parameters from the String Message.
* @param String Message: (Input)the original Message received form the Serial port
* e.g. 700@4,2017/2/2,11:55#
*/
void ExtractCurrentTime(char *Message) {
	//debugPrint("In the ExtractCurrentTime Function", 10, true); // delay(100);
	//Serial.print(Message); delay(300);
	//when extract the current time and save the "Saved" Status
	//Save the current time to eeprom
	//char Temp[17];

	strSplitNo(Message, firstLine, sizeof(firstLine), '@', 1);
	//debugPrint(firstLine, 10, true);
	int FirstEND = strcspn(firstLine, "#");
	firstLine[FirstEND] = 0;

	debugPrint(firstLine, 10, true);
	//extract weekday 

	//the content in the array firstLine should have the format that
	//"1,2017/1/23,12:48"
	char C_WeekDay[2];
	//memset(C_WeekDay, 0, sizeof(C_WeekDay));//clean the char array up

	strSplitNo(firstLine, C_WeekDay, sizeof(C_WeekDay), ',', 0);//get the 1 from "1,2017/1/23,12:48"

	debugPrint(C_WeekDay, 10, true);
	currentTime.Current_WeekDay = atoi(C_WeekDay);

	//find the 12:48 and saved it into C_HourMinute
	char C_HourMinuteSec[9];
	char C_HM[3];
	//memset(C_HourMinute, 0, sizeof(C_HourMinute));
	//memset(C_HM, 0, sizeof(C_HM));
	strSplitNo(firstLine, C_HourMinuteSec, sizeof(C_HourMinuteSec), ',', 2);//get the 12:48 from "1,2017/1/23,12:48"

																			//print the result out and transfer them into integers
																			//save the 12 into C_Hour
	strSplitNo(C_HourMinuteSec, C_HM, sizeof(C_HM), ':', 0);
	currentTime.Current_Hour = atoi(C_HM);
	debugPrint(C_HM, 5, false); debugPrint(":", 5, false);
	//save the 48 into C_Minute
	strSplitNo(C_HourMinuteSec, C_HM, sizeof(C_HM), ':', 1);
	currentTime.Current_Minute = atoi(C_HM);
	debugPrint(C_HM, 5, true);
	//save the 33 into Current_Second
	strSplitNo(C_HourMinuteSec, C_HM, sizeof(C_HM), ':', 2);
	currentTime.Current_Second = atoi(C_HM);
	debugPrint(C_HM, 5, true);

	//strSplitNo(Message, Temp, sizeof(Temp), '#', 0);//subtract the "#" and the characters after it
	//strSplitNo(Temp, firstLine, sizeof(firstLine), '@', 1);//subtract the "@" and the characters before it

	char C_YearMonthDay[10];
	char C_Year[5];
	char C_Month[3];
	char C_Day[3];
	//find the 2017/1/23 and saved it into C_YearMonthDay
	strSplitNo(firstLine, C_YearMonthDay, sizeof(C_YearMonthDay), ',', 1);
	//find the 2017 and save it into C_Year
	strSplitNo(C_YearMonthDay, C_Year, sizeof(C_Year), '/', 0);
	debugPrint(C_Year, 5, false); debugPrint("/", 5, false);
	//find the 1 and save it into C_Month
	strSplitNo(C_YearMonthDay, C_Month, sizeof(C_Month), '/', 1);
	debugPrint(C_Month, 5, false); debugPrint("/", 5, false);
	//find the 23 and save it into C_Day
	strSplitNo(C_YearMonthDay, C_Day, sizeof(C_Day), '/', 2);
	debugPrint(C_Day, 5, true); //debugPrint(":", 5, true);
								//Serial.print(C_YearMonthDay); delay(100);
	currentTime.Current_Year = atoi(C_Year);
	currentTime.Current_Month = atoi(C_Month);
	currentTime.Current_DayOfMonth = atoi(C_Day);

	debugPrint("Out of the ExtractCurrentTime Function", 15, true);
	//  eeprom_read_block((void*)&CurrentTime, (void*)0, sizeof(ScheduleSaves));
}

void printTimeDateDbg(struct timeDate *td) {
	debugPrint("Time: ", 10, false);
	debugPrint(td->Current_Hour, 5, false); debugPrint(":", 5, false); debugPrint(td->Current_Minute, 5, false);
	debugPrint(":", 5, false); debugPrint(td->Current_Second, 5, false); debugPrint(", ", 5, false);
	debugPrint(td->Current_WeekDay, 5, false); debugPrint(", ", 5, false); debugPrint(td->Current_DayOfMonth, 5, false);
	debugPrint("/", 5, false); debugPrint(td->Current_Month, 5, false); debugPrint("/", 5, false); debugPrint(td->Current_Year, 5, true);

}
/**
* Name: SaveWorkingdaySchedules
* Description: Save the working day(Mon. - Fri.) Schedules into eeprom.
* @param String Message: (Input)the original Message received form the Serial port
* @param int WD_NofSchedule: (Input)The number of the working day schedules for constructing the array
*/
void SaveDaySchedules(char *Message) {
	//new version
	//Input parameter format: 711@12:00|50,13:00|50,14:00|50# 
	//the maximum number of the schedules should be 10, then the length of the array created here
	//should be no more than 100
	//debugPrint("In the SaveDaySchedules Function", 10, true);

	//memset(firstLine, 0, sizeof(firstLine));
	strSplitNo(Message, firstLine, sizeof(firstLine), '@', 0);
	int Header = atoi(firstLine);

	int PiecesCount;
	PiecesCount = strSplitCount(Message, ',');
	//debugPrint(PiecesCount, 5, true);
	char *C_SchedulePiece = firstLine;
	char C_HourMinute[6];
	char C_BeginHour[3];
	char C_BeginMinute[3];
	char C_Percentage[4];

	struct ScheduleSaves *Temp_ArrayWDSchedules = new struct ScheduleSaves[PiecesCount];
	//WD_NumberOfSchedules = PiecesCount;
	//Serial.print(WD_NumberOfSchedules); delay(800);

	for (int i = 0; i < PiecesCount; i++)
	{
		C_SchedulePiece = firstLine;
		//Save the 12:00|50(one section) into C_SchedulePiece
		debugPrint(i, 5, false); debugPrint(": ", 2, false);
		strSplitNo(Message, C_SchedulePiece, sizeof(firstLine), ',', i);
		if (i == 0) C_SchedulePiece += strcspn(C_SchedulePiece, "@") + 1; // to skip the header code if the first piece
		if (i == PiecesCount - 1) C_SchedulePiece[strcspn(C_SchedulePiece, "#")] = 0; // to skip the "#" if the last piece
		debugPrint(C_SchedulePiece, 5, true);
		//Save the percentage message into C_Percentage
		strSplitNo(C_SchedulePiece, C_Percentage, sizeof(C_Percentage), '|', 1);
		//Save the Hour and Minute message into C_HourMinute
		strSplitNo(C_SchedulePiece, C_HourMinute, sizeof(C_HourMinute), '|', 0);
		//Save the Hour message into C_HM
		strSplitNo(C_HourMinute, C_BeginHour, sizeof(C_BeginHour), ':', 0);
		//Save the Minute message into C_BeginMinute
		strSplitNo(C_HourMinute, C_BeginMinute, sizeof(C_BeginMinute), ':', 1);

		//save this schedule into the eepo
		//struct ScheduleSaves temp;
		Temp_ArrayWDSchedules[i].StartTimeHour = atoi(C_BeginHour);
		Temp_ArrayWDSchedules[i].StartTimeMinute = atoi(C_BeginMinute);
		Temp_ArrayWDSchedules[i].OpeningPercentage = atoi(C_Percentage);

		//Temp_ArrayWDSchedules[i] = temp;
		//SaveIntoEEPROM(temp, i, IsBeingSaved, WD_ScheduleParts, 1);
	}
	//delete(ArrayWDSchedules); // delete the previous array from memory
	//ArrayWDSchedules = Temp_ArrayWDSchedules;
	// 711 => 1 (Sunday), 712 => 2 (Monday), 713 => 3 (Tuesday), ... 717 => 7 (Saturday)
	SaveDayScheduleIntoEEPROM(Temp_ArrayWDSchedules, Header - 720, PiecesCount);
	delete(Temp_ArrayWDSchedules);
	if ((Header - 720) == currentTime.Current_WeekDay) loadTodaysSchedule();
	//testing
	/*Serial.print(ArrayWDSchedules[0].OpeningPercentage);  delay(300);
	Serial.print(ArrayWDSchedules[0].StartTimeHour);  delay(300);
	Serial.print(ArrayWDSchedules[0].StartTimeMinute);  delay(300);*/
	//debugPrint("Out of the SaveDaySchedules Function", 10, true);
}

/**
Used to create the initial schedule at the first use of the Arduino
**/
void createInitialSchedules(bool overWrite) {
	int save;
	eeprom_read_block((void*)(&save), (void*)(0), sizeof(int));
	//Serial.print("Save Code: "); Serial.println(save); delay(10);
	if (!overWrite && (save == InitialScheduleSaveCode || save == (InitialScheduleSaveCode+5))) return; // Schedule already saved. just return
	struct ScheduleSaves Schedule;  // Open the vent all day
	Schedule.OpeningPercentage = 100;
	Schedule.StartTimeHour = 0;
	Schedule.StartTimeMinute = 0;
	lastUpdateTime.Current_Hour = -1;
	for (int i = 1; i <= 7; i++)
		SaveDayScheduleIntoEEPROM(&Schedule, i, 1);
	// Write the flag to indicate initial schedule is saved
	save = InitialScheduleSaveCode; // Indicate initial schedule is saved
	eeprom_write_block((const void*)(&save), (void*)(0), sizeof(int));
	char c = 0;	// No name set yet
	eeprom_write_block((const void*)(&c), (void*)(sizeof(int)), 1);
}

/**
* Name: SaveIntoEEPROM
* Description: The exact general saving operations, including saving WorkingDays, Weekend and the Saving Flag as well as the
* number of the WD&WE Schedules
* @param struct ScheduleSaves TEMP: (Input)The Structure of SchedulesSave
* @param int index: (Input)The index number of the saving the structure to EEPROM only used for case 1 & 2
* @param int Save: (Input)Message of the saving flag 100 means saved others means not saved
* @param int NumberOfSchedules: (Input)Integer for the # of Schedules
* @param int Operations: (Input)Integer for choosing which operations to perform(switch case)
* 1: saved Working days Schedules; 2: saved Weekend Schedules; 3: saved ISSaved flag
* 4: saved the Number of the WD Parts; 5: saved the Number of the WE Parts
*/
void SaveDayScheduleIntoEEPROM(struct ScheduleSaves *Schedule, int day, int NumberOfSchedules) {
	// Save the save code
	int save; int sizeOfInt = sizeof(int);
	int start = 0; // skip for the save code
	eeprom_read_block((void*)(&save), (void*)(start), sizeOfInt);
	if (save == InitialScheduleSaveCode) {
		save = InitialScheduleSaveCode+5; // Indicate a user (not initial) schedule is saved
		eeprom_write_block((const void*)(&save), (void*)(start), sizeOfInt);
	}
	start += sizeOfInt + maxNameLength;	// Added characters for the Vent Name

										// Write the last update dateTime
	struct timeDate tmpT;
	eeprom_read_block((void*)(&tmpT), (void*)(start), sizeof(lastUpdateTime));
	if (tmpT.Current_Hour != currentTime.Current_Hour || tmpT.Current_Minute != currentTime.Current_Minute
		|| tmpT.Current_DayOfMonth != currentTime.Current_DayOfMonth)
	{
		copyTimeDate(&currentTime, &lastUpdateTime);
		eeprom_write_block((const void*)(&lastUpdateTime), (void*)(start), sizeof(lastUpdateTime));
		//Serial.print("Current time:");  printTimeDate(&currentTime);
		//Serial.print("copied time:");  printTimeDate(&lastUpdateTime); delay(100);
	}
	start += sizeof(lastUpdateTime);
	//Serial.print("sizeof(lastUpdateTime): "); Serial.print(sizeof(lastUpdateTime));

	// will write the no of schedules then the actual schedules 
	/* The first 2 bytes are saves to store InitialScheduleSaveCode which means the initial schedule is saved, 
	or InitialScheduleSaveCode+5 means user schedule is saved*/
	start += (day - 1) * (sizeOfInt + ScheduleSize * MaxDaySchedules);  // Assume the day has max of MaxDaySchedules schedules to use for memory block of the day + the actual count
	eeprom_write_block((const void*)&NumberOfSchedules, (void*)(start), sizeOfInt);
	start += sizeOfInt;
	for (int i = 0; i < NumberOfSchedules; i++) {
		eeprom_write_block((const void*)&Schedule[i], (void*)(start), ScheduleSize);
		start += ScheduleSize;
	}

}


struct ScheduleSaves * ReadDayScheduleFromEEPROM(int day, int *pNumberOfSchedules) {

	int sizeOfInt = sizeof(int);
	int start = sizeOfInt + maxNameLength;  // skip the first code and the Vent name
	eeprom_read_block((void*)(&lastUpdateTime), (void*)(start), sizeof(lastUpdateTime));
	start += sizeof(lastUpdateTime);

	// The first 2 bytes are saves to store InitialScheduleSaveCode which means the initial schedule is saved, or 200 for user schedule
	start += (day - 1) * (sizeOfInt + ScheduleSize * MaxDaySchedules);  // Assume the day has max of MaxDaySchedules schedules to use for memory block of the day + the actual count

	eeprom_read_block((void*)(pNumberOfSchedules), (void*)(start), sizeOfInt);

	struct ScheduleSaves *Schedules = new struct ScheduleSaves[*pNumberOfSchedules];
	start += sizeOfInt;
	for (int i = 0; i < *pNumberOfSchedules; i++) {
		eeprom_read_block((void*)&Schedules[i], (void*)(start), ScheduleSize);
		start += ScheduleSize;
	}
	return Schedules;
}


/**
* Name: CheckWorkingDaySchedule
* Description: check the working day schedule every minute.
* @param int NofScheduleParts: the number of the working day schedules
* @param struct ScheduleSaves* Temporary: a reference of a return structure, after getting the section in the eeprom
*                     read the exact structure from the eeprom and return
* return: the section which schedule period the current time located in (the index of the schedule array)
*/
//struct ScheduleSaves* ArrayWDSchedules;
ScheduleSaves * getCurrentScheduleSection(int NofScheduleParts, struct ScheduleSaves * Schedules) {
	//debugPrint("In CheckSchedule Function", 10, true);
	int currentT = currentTime.Current_Hour * 60 + currentTime.Current_Minute;
	for (int k = NofScheduleParts - 1; k >= 0; k--) {
		int beginT = Schedules[k].StartTimeHour * 60 + Schedules[k].StartTimeMinute;
		if (currentT >= beginT) {
			printSchedule(&Schedules[k]);
			return &Schedules[k];
		}

	}
	return 0;
}



//this function will be called every 1 minute and tell the remain time
//parameter: true means Working days; false means weekend
//return: the opening percentage for this period
int CheckScheduleModule() {

	//debugPrint("In the Check Schedule Function", 20, true);
	printTimeDateDbg(&currentTime);

	struct ScheduleSaves * currentSchedule = getCurrentScheduleSection(Today_NumberOfSchedules, TodaySchedules);
	//}
	//if (currentSchedule)
	//debugPrint(currentSchedule->OpeningPercentage, 10, true);
	//Serial.print("Out of the CheckScheduleModule Function");  delay(200);
	return currentSchedule ? currentSchedule->OpeningPercentage : -1;

}

/**
Converts the given number to 2 characters string padded by 0's
*/
void convertTo2digitsString(int number, char * out) {
	sprintf(out, "%d", number);
	if (strlen(out) < 2) sprintf(out, "%d%d", 0, number);
	return;
}


/**
* Name: DecideGeneralOperations
* Description: Decide which operation should be perform based on the input parameter header.(general operations and saving)
* @param int Header: Header of the operation
*/
void DecideGeneralOperations(char *Receive) {
	int Header = FindHeader(Receive);
	if (!AccessOK && (Header != 100 && Header != 10))
		return;   // Must be authenticated to do all actions except 100
	switch (Header) {
		case 10:	// Restart Arduino to program it
		{
			//perform the software reset, mode can be STANDARD, ALTERNATIVE or SKETCH
			//softwareReset(STANDARD);
			Serial.println(F("015@Restart#")); delay(100);
			pinMode(resetPin, OUTPUT);      // sets the digital pin as output
			digitalWrite(resetPin, LOW); // reset the Arduino.0 

		}
		case 30:	// Get Code version number
		{
			Serial.print(F("035@"));
			Serial.print(CodeVersion);
			Serial.println("#"); delay(10);
		}
		case 50: // 050@WakeUp#
		{
			int AT = strcspn(Receive, "@");
			int END = strcspn(Receive, "#");
			strncpy(firstLine, Receive + AT + 1, END - AT - 1);
			firstLine[END - AT - 1] = 0;
			if (strncmp("WakeUp", firstLine, strlen("WakeUp")) == 0)  // WakeUp
				Serial.println(F("055@#")); delay(10);
			break;
		}
		case 60://060@Disconnected#
		{
			int AT = strcspn(Receive, "@");
			int END = strcspn(Receive, "#");
			strncpy(firstLine, Receive + AT + 1, END - AT - 1);
			firstLine[END - AT - 1] = 0;
			if (strncmp("Disconnected", firstLine, strlen("Disconnected")) != 0) return;
			AccessOK = false;
			Disconnected = true;  // wrong password. Do nothing
			Serial.println(F("065@DisconnectedReceived#")); delay(10);
			break;
		}
		case 100://App password 100@SmartEnergy0550#
		{
			int AT = strcspn(Receive, "@");
			int END = strcspn(Receive, "#");
			strncpy(firstLine, Receive + AT + 1, END - AT - 1);
			firstLine[END - AT - 1] = 0;
			AccessOK = false;  //false
			if (strncmp(appPassword, firstLine, strlen(appPassword)) != 0) return;  // wrong password. Do nothing
			AccessOK = true;
			Serial.println(F("105@AccessOK#")); delay(10);

			LED_FLASH(5, 500);			// Indicate that it authorized

			break;
		}
		case 300://Blink a LED on the device
		{
			//debugPrint("Headcode is 300", 10, true);
			//debugPrint("In the BlinkLED Function: ", 15, false);
			//receiveOk = false;
			LEDStatus = HIGH;
			blinkCount = 20;
			blinkMSec = 500;

			Serial.print("310@#"); delay(10);
			break;
		}
		case 400://Update device name
		{
			//debugPrint("Headcode is 400", 10, true);
			int AT = strcspn(Receive, "@");
			int END = strcspn(Receive, "#");
			char name[maxNameLength];
			strncpy(name, Receive + AT + 1, END - AT - 1);
			name[END - AT - 1] = 0;

			eeprom_write_block((const void*)(name), (void*)(sizeof(int)), maxNameLength);

			//BLTNameUpdated = true;
			eeprom_read_block((void*)(name), (const void*)(sizeof(int)), maxNameLength);
			Serial.print("405@"); delay(5);
			Serial.print(name); delay(10);
			Serial.println("#"); delay(5);
			break;
		}
		case 410://Read device name
		{
			//debugPrint("Headcode is 410", 10, true);
			char name[maxNameLength];
			eeprom_read_block((void*)(name), (const void*)(sizeof(int)), maxNameLength);
			Serial.print(F("415@")); delay(5);
			Serial.print(name); delay(10);
			Serial.println(F("#")); delay(5);
			break;
		}
		case 500://Update date and time
		{
			//debugPrint("Headcode is 500", 10, true);
			ExtractCurrentTime(Receive);
		
			CR.setModuleClock(currentTime.Current_Second, currentTime.Current_Minute, currentTime.Current_Hour, \
				currentTime.Current_WeekDay, currentTime.Current_DayOfMonth, currentTime.Current_Month, \
				currentTime.Current_Year);
		
			LastMsgTime = millis();
			OldTimer = NewTimer - currentTime.Current_Second * 1000;
			//SavedTime = true;
			Serial.print(F("505@#"));
			printTime(&currentTime);
			Serial.print(F("#"));
			delay(10);
			break;
		}
		case 507://Read date and time
		{
			//debugPrint("Headcode is 507", 10, true);
			Serial.print(F("508@"));
			CR.syncClockFromModule(&currentTime);	// Read the clock from the module
			printTime(&currentTime);
			Serial.print(F("#"));
			delay(10);
			break;
		}
		case 510://Request the Battery Voltage of the vent
		{
			//Serial.print("Headcode is 510"); delay(100);
			Serial.print(F("515@"));
			Serial.print(fReadVcc());
			Serial.print(F("#"));
			delay(10);
			break;
		}
		case 530://Request the temperature of the vent
		{
			//Serial.print("Headcode is 530"); delay(100);
			// = ReadTemp(); //delay(50);
			if(current_temp < -100) current_temp = ReadTemp();
			Serial.print(F("535@"));
			if (current_temp > -100) Serial.print(current_temp);
			Serial.print(F("#"));
			delay(10);
			//ReadTemp();
			break;
		}
		case 600://Read the Vent open percentage 
		{
			//Serial.print("Headcode is 600"); delay(100);
			Serial.print(F("605@"));
			Serial.print(TodaySchedules->OpeningPercentage);
			Serial.print(F("#"));
			delay(10);
			break;
		}
		case 620://Open or close the Vent
		{
			if (MotorVoltLow) { Serial.print(F("Battery is low#")); delay(10); }
			else
			{
				//debugPrint("Headcode is 620", 10, true);
				//Serial.print("In the OpenCloseVent Function"); delay(100);
				int AT = strcspn(Receive, "@");
				int END = strcspn(Receive, "#");
				strncpy(firstLine, Receive + AT + 1, END - AT - 1);
				firstLine[END - AT - 1] = 0;
				int NEWOpeningPercentage = atoi(firstLine);
				debugPrint(NEWOpeningPercentage, 10, true);
				OpenCloseVent(NEWOpeningPercentage);
				//debugPrint("Old Schedule:", 10, true);
				struct ScheduleSaves * currentSchedule = getCurrentScheduleSection(Today_NumberOfSchedules, TodaySchedules);
				if (currentSchedule)
				{
					currentSchedule->OpeningPercentage = NEWOpeningPercentage;    // Over write the current open percentage */
					//debugPrint("New Opening percentage: ", 10, false);
					debugPrint(currentSchedule->OpeningPercentage, 10, false);
					debugPrint(" %", 5, true);
				}
				Serial.print(F("625@#")); delay(10);
			}
			break;
		}
		case 650://Disable Schedule 
		{
			//Serial.print("Headcode is 600"); delay(100);
			DisableSchedule = true;
			Serial.print(F("655@"));
			Serial.print(F("Schedule Disabled"));
			Serial.print(F("#"));
			delay(10);
			break;
		}
		case 660://Enable Schedule 
		{
			//Serial.print("Headcode is 600"); delay(100);
			DisableSchedule = false;
			Serial.print(F("665@"));
			Serial.print(F("Schedule Enabled"));
			Serial.print(F("#"));
			delay(10);
			break;
		}
		case 700://Request last update time of schedule.  
		{
			//Serial.print("Headcode is 700"); delay(100);

			Serial.print(F("705@"));
			printTime(&lastUpdateTime);
			Serial.print(F("#"));
			delay(50);
			break;
		}
		case 711://read a day schedule
		case 712://read a day schedule
		case 713://read a day schedule
		case 714://read a day schedule
		case 715://read a day schedule
		case 716://read a day schedule
		case 717:
		{
			int NumberOfSchedules;
			struct ScheduleSaves* Schedule = ReadDayScheduleFromEEPROM(Header - 710, &NumberOfSchedules);
			Serial.print(Header); Serial.print(F("@"));
			char value[3];
			for (int i = 0; i < NumberOfSchedules; i++) {
				convertTo2digitsString(Schedule[i].StartTimeHour, value);
				Serial.print(value); Serial.print(":");
				convertTo2digitsString(Schedule[i].StartTimeMinute, value);
				Serial.print(value); Serial.print("|");
				Serial.print(Schedule[i].OpeningPercentage);
				if (i < NumberOfSchedules - 1) Serial.print(",");
				delay(10);
			}
			Serial.print(F("#")); delay(50);
			delete(Schedule);
			break;
		}
		case 721:	//save a day schedule
		case 722:	//save a day schedule
		case 723:	//save a day schedule
		case 724:	//save a day schedule
		case 725:	//save a day schedule
		case 726:	//save a day schedule
		case 727:	//save a day schedule
		{
			SaveDaySchedules(Receive);
			int NumberOfSchedules;
			struct ScheduleSaves* Schedule = ReadDayScheduleFromEEPROM(Header - 720, &NumberOfSchedules);
			Serial.print(Header); Serial.print(F("@"));
			char value[3];
			for (int i = 0; i < NumberOfSchedules; i++) {
				convertTo2digitsString(Schedule[i].StartTimeHour, value);
				Serial.print(value); Serial.print(":");
				convertTo2digitsString(Schedule[i].StartTimeMinute, value);
				Serial.print(value); Serial.print("|");
				Serial.print(Schedule[i].OpeningPercentage);
				if (i < NumberOfSchedules - 1) Serial.print(",");
				delay(10);
			}
			Serial.print(F("#")); delay(50);
			delete(Schedule);
			break;
		}
		case 900:	// Requesting Arduino Data batch Headers count 
		{
			Serial.print(F("905@"));
			Serial.print(DataLogging.getHeadersCount());
			Serial.print(F("#"));
			delay(5);
			break;
		}
		case 910:	// Requesting Arduino Data Log Headers count 
		{
			Serial.print(F("915@"));
			Serial.print(DataLogging.getLastHeadersNumber());
			Serial.print(F("#"));
			delay(5);
			break;
		}
		case 920:	// Requesting read out of header batch number
		{
			int AT = strcspn(Receive, "@");
			int END = strcspn(Receive, "#");
			strncpy(firstLine, Receive + AT + 1, END - AT - 1);
			firstLine[END - AT - 1] = 0;
			int number = atoi(firstLine);
			Serial.print(F("925@"));
			DataLogging.sendOutHeader(number);
			Serial.print(F("#"));
			delay(5);
			break;
		}
		case 930:	// Requesting samples data from batch number
		{
			int AT = strcspn(Receive, "@");
			int END = strcspn(Receive, "#");
			strncpy(firstLine, Receive + AT + 1, END - AT - 1);
			firstLine[END - AT - 1] = 0;
			int number = atoi(firstLine);
			Serial.print(F("935@"));
			DataLogging.sendOutBatchData(number);
			Serial.print(F("#"));
			delay(5);
			break;
		}
	}
}


/**
* Name: IncreaseCurrentTime
* Description: it is a clock the unit for the clock is 1 minute.
*/
void IncreaseCurrentTime() {
	if (currentTime.Current_Minute == 59) {
		currentTime.Current_Minute = 0;
		if (currentTime.Current_Hour == 23) {
			currentTime.Current_Hour = -1;
			if (currentTime.Current_WeekDay == 7) {
				currentTime.Current_WeekDay = 0;
			}
			currentTime.Current_WeekDay++;
			runEveryDay();

			if (currentTime.Current_Month <= 7) {
				if (currentTime.Current_Month % 2 == 1) {
					if (currentTime.Current_DayOfMonth == 31) {
						currentTime.Current_DayOfMonth = 0;
						currentTime.Current_Month++;
					}
					currentTime.Current_DayOfMonth++;
				}
				else {
					if (currentTime.Current_Month == 2) {
						if (((currentTime.Current_Year % 4 == 0) && (currentTime.Current_Year % 100 != 0)) || (currentTime.Current_Year % 400 == 0)) {
							if (currentTime.Current_DayOfMonth == 29) {
								currentTime.Current_DayOfMonth = 0;
								currentTime.Current_Month++;
							}
							currentTime.Current_DayOfMonth++;
						}
						else {
							if (currentTime.Current_DayOfMonth == 28) {
								currentTime.Current_DayOfMonth = 0;
								currentTime.Current_Month++;
							}
							currentTime.Current_DayOfMonth++;
						}
					}
					else {
						if (currentTime.Current_DayOfMonth == 30) {
							currentTime.Current_DayOfMonth = 0;
							currentTime.Current_Month++;
						}
						currentTime.Current_DayOfMonth++;
					}
				}
			}
			else {
				if (currentTime.Current_DayOfMonth % 2 == 0) {
					if (currentTime.Current_DayOfMonth == 31) {
						currentTime.Current_DayOfMonth = 0;
						if (currentTime.Current_Month == 12) {
							currentTime.Current_Month = 0;
							currentTime.Current_Year++;
						}
						currentTime.Current_Month++;
					}
					currentTime.Current_DayOfMonth++;
				}
				else {
					if (currentTime.Current_DayOfMonth == 30) {
						currentTime.Current_DayOfMonth = 0;
						currentTime.Current_Month++;
					}
					currentTime.Current_DayOfMonth++;
				}
			}
		}
		currentTime.Current_Hour++;
		runEveryHour();
	}
	else {
		currentTime.Current_Minute++;
		runEveryMinute();

	}
	/*if (Current_WeekDay <= 5) {
	ISWD = true;
	}
	else {
	ISWD = false;
	}*/
}

// Sleeps the Arduino for the given seconds (1, 2, 4, or 8)
void Arduino_sleep(short period, bool attach_interrupt = true) {
	////////////////////////// Power Saving //////////////////////
	// delay(200); going to sleep here
	debugPrint("Going to sleep", 50, true);
	int shift;
	/* Sleeping 1 second  SLEEP_1S shift time by 1067 */
	/* Sleeping 2 seconds SLEEP_2S shift time by 2067 */
	/* Sleeping 4 seconds SLEEP_4S shift time by 4270 */
	/* Sleeping 8 seconds SLEEP_8S shift time by 8200 */

	// Attach Interrupt 
	//if (attach_interrupt)
		//attachInterrupt(digitalPinToInterrupt(BLTinterruptPin), pin_ISR, FALLING);

	switch (period) {
	case 1:
		LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
		shift = 1067;
		break;
	case 2:
		LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
		shift = 2067;  // 2068; // 2070;	//2065, 2055, 2073. 2085, 2100, 2128
		break;
	case 4:
		LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
		shift = 4270;
		break;
	case 8:
		LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
		shift = 8200;
		break;
	default:
		LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
		shift = 2067;
		break;
	}
	OldTimer = OldTimer > shift ? OldTimer - shift : 0L;
	LastBLTOn = LastBLTOn > shift ? LastBLTOn - shift : 0L;
	LastMsgTime = LastMsgTime > shift ? LastMsgTime - shift : 0L;

	//// Check for Interrupt /////
	/*if (attach_interrupt) {
		detachInterrupt(digitalPinToInterrupt(BLTinterruptPin));
		if (InterruptedOnce)
		{
			//delay(10);
			LastMsgTime = millis() - MaxLastMessageTime + 1000;	//100 Keep awake to see if there is any messages
			debugPrint("** Interrupted", 50, true);
			//{Serial.print("55@#"); delay(50); }
			//LastMsgTime = millis();
			InterruptedOnce = false;
		}
	}*/

	////////////////////////////////////////////////////////////////////// back from sleep here
}
void loop() {
	NewTimer = millis();

	//debugPrint(" before 20 sec", 10, true);
	//debugPrint((NewTimer - LastMsgTime)/1000, 10, true);
	if (NewTimer < LastMsgTime) {
		Serial.print("Error LastMsgTime= ");
		Serial.print(LastMsgTime);
		Serial.print(", NewTimer= ");
		Serial.println(NewTimer); delay(20);
		LastMsgTime = NewTimer;
	}

	if (NewTimer - LastMsgTime >= MaxLastMessageTime)
	{
		debugPrint("Connection Time is more than 60sec, BLT off, and seeping ...", 50, true);

		//InterruptedOnce = false;
		AccessOK = false; 

		/*if (canBLETurnOff && (NewTimer - LastBLTOn >= MinBLTupTime))   // BLE off
		{
			debugPrint("Time for High is: ", 50, false);
			debugPrint((NewTimer - LastBLTOn) / 1000, 50, true);
			BLT_PR_ON = false;
			//Serial.println("Bluetooth Off"); delay(25);
			Serial.println("AT+SLEEP"); delay(20);	// Sleep the BLE
						//digitalWrite(BLTPower, LOW); //delay(200); //LOW
						//digitalWrite(test, LOW); //delay(200);
			debugPrint("Bluetooth is LOW ", 50, false);
			//debugPrint((millis() - LastBLTOn)/1000, 50, true);
			//LastBLTOn = LastBLTOn + MinBLTupTime;
			//LastBLTOff = millis();
		}*/		
		bool connected = digitalRead(BLTstatusPin);
		/*BLT_PR_ON = !BLT_PR_ON || connected; 
		//digitalWrite(BLTPower, BLT_PR_ON);	// Turn BLE ON/OFF
		if (BLT_PR_ON && !connected) {
			delay(20);
			Serial.println(F("AT+SLEEP")); 	// Sleep the BLE
			delay(100);		// Wait for BLE response 
		}
		digitalWrite(ClockPowerPin, BLT_PR_ON); // Power on the clock for testing <<<<<< remove
		delay(10);
		digitalWrite(ClockPowerPin, LOW); // Power on the clock for testing <<<<<< remove
		delay(100);
		digitalWrite(ClockPowerPin, HIGH); // Power on the clock for testing <<<<<< remove
		delay(10);
		digitalWrite(ClockPowerPin, LOW); // Power on the clock for testing <<<<<< remove

		if (!connected) {	// Sleep Arduino only when there is no connection
			if (BLT_PR_ON)
				Arduino_sleep(1 /*BLTOnTime* /, false);
			else
				Arduino_sleep(4/*BLTOffTime* /, false);
		}*/

		if (!connected) {	// Sleep Arduino only when there is no connection
			if (InterruptedOnce)
			{
				delay(20);
				Serial.println(F("AT+SLEEP")); 	// Sleep the BLE
				delay(100);		// Wait for BLE response 
				InterruptedOnce = false;
			}
			if(!LEDStatus)	// Don't sleep if LED is ON
				Arduino_sleep(4/*BLTOffTime*/, false);			
		}
	}
	
	////////////////////////// Read Battery Voltage //////////////////////
	//DecideBatteryLevel( float Vcc);
	/////////////////////////////////////////////////////////////////////////

	if (NewTimer - LastMsgTime >= 1000L)	rcvIndex = 0;	// Discard incomplete messages after 0.6 sec
	char temp = 0;
	while (Serial.available())
	{
		BLEisSleep = 10;
		temp = Serial.read();
		//if (temp != 'O' || temp != 'K')
		//Serial.print(temp); 
		
		if (!isprint(temp)) { // Check to make sure the received characters are valid
			//Serial.print(F("Not Printable# ")); Serial.println((int)temp);  delay(50);
			if (temp != 10 && temp != 13) rcvIndex = 0;	// Only skip LF & CR
			continue;
		}
		Receive[rcvIndex++] = temp;

		if (temp == '#') {
			Receive[rcvIndex] = '\0';
			rcvIndex = 0;
			receiveOk = true;
			//if (InterruptedOnce) Serial.print("Interrupt WakeUp");
			LastMsgTime = millis();
			break;
		}
		// Wait if no data is available, max 10 mSec
		for (uint8_t i = 0; (i < 10) && !Serial.available(); i++) 
			delay(1);
	}

	// Read AT commands from Blue-tooth ///
	// If this value is set to 1, when link ESTABLISHED or LOSTED 
	// module will send OK+CONN or OK+LOST string through UART.
	if (Receive[0] == 'O' && Receive[1] == 'K')
	{
		Receive[rcvIndex] = '\0';
		rcvIndex = 0;
		//if (strncmp(Receive, "OK+LOST", strlen("OK+LOST")) == 0) BLTnotConnected = true;
		//receiveOk = true;
	}
	if (Receive[0] == '+')
	{
		Receive[rcvIndex] = '\0';
		rcvIndex = 0;
		if (strncmp(Receive, "+SLEEP:OK", strlen("+SLEEP:OK")) == 0) BLEisSleep = 0;
		//Serial.print("Slept");
		//receiveOk = true;
	}

	
	//  serialPrint("out of listening loop");
	//  delay(300);
	if (receiveOk == true) {
		receiveOk = false;
		debugPrint("\nMessage Received: ", 10, false);
		debugPrint(Receive, 100, true);
		//Watchdog.reset();
		DecideGeneralOperations(Receive);
		///////
	}

	BlinkLED();

	//debugPrint(NewTimer, 10, true);
	//debugPrint(OldTimer, 10, true);
	currentTime.Current_Second = (int)((NewTimer - OldTimer) / 1000);

	if ((NewTimer - OldTimer) >= 60000L - 1) {	// Control clock increment

		//debugPrint("\nStart Checking the Schedule ...", 10, true);
		//debugPrint("Battery Voltage: ", 10, false);
		//debugPrint(fReadVcc(), 10, false); debugPrint("Volt", 10, true);
		//debugPrint(ReadTemp(), 10, false); debugPrint("C", 10, true);
		//check the schedule from the eepom memory module
		//if Current_Hour equals 99 means the time information is not saved 
		//if(Current_Hour != 99){
		OldTimer = NewTimer;
		//Watchdog.reset();
		IncreaseCurrentTime(); 
		//Serial.print(ArrayWDSchedules[0].OpeningPercentage); delay(100);
		//Watchdog.reset();
		if (!DisableSchedule)
		{
			int percentage = CheckScheduleModule();
			if (percentage == -1)
			{
				debugPrint("No new activities in the Schedule,\nLast opening Percentage is ", 10, false);
				debugPrint(TodaySchedules->OpeningPercentage, 10, false);
				debugPrint(" %", 5, true);
			}
			else OpenCloseVent(percentage);
			//debugPrint(percentage, 10, true);

			debugPrint("End of Checking the Schedule.", 10, true);
		}
	}

	//Watchdog.reset();
}



// Checks if BLE is not sleep to send it to sleep
void checkBLEsleep() {
	return;
	//if ((currentTime.Current_Minute % 10) != 0) return;	// run every 10 min
		////////////////////////// Sleep the BLE //////////////////////////
	if (NewTimer - LastMsgTime < MaxLastMessageTime) return; // Make sure we are not in receive mode
		
	if (!BLEisSleep) return;	// Stop when the counter = 0
		
	Serial.println(F("AT+SLEEP"));  // Sleep the BLE
	BLEisSleep--;
	delay(10);		
	//Serial.println("AT"); delay(10);		// Check if the BLE is active and not connected 	
}

// Reads the temperature every minute 
void readTempTimely() {
	if ((currentTime.Current_Minute % 2) != 0) return;
	current_temp = ReadTemp();
	if (current_temp < -100) current_temp = ReadTemp();
	//Serial.print(F("Temp: ")); Serial.println(current_temp); delay(10);
	if (current_temp > -100) {	// To skip the error in measuring them (-999)
		temperature_low = temperature_low < current_temp ? temperature_low : current_temp;
		temperature_high = temperature_high > current_temp ? temperature_high : current_temp;
		//Serial.print(F("Temp H: ")); Serial.println(temperature_low);
		//Serial.print(F("Temp L: ")); Serial.println(temperature_high); delay(50);
	}
}

// This function runs every 1 minute
void runEveryMinute() {
	checkBLEsleep();
	readTempTimely();
	
}

// This function runs every 1 Hour
void runEveryHour() {
	//if ((currentTime.Current_Hour % 6) == 0)	
	CR.syncClockFromModule(&currentTime);
	DataLogging.logSample_Temperature(temperature_high, temperature_low);	// Stores the given High and Low temperature sample into the log array 	
	temperature_low = 1000.0;
	temperature_high = -1000.0;
}

// This function runs every 1 day, at 12 am (beginning of the day)
void runEveryDay() {	
	loadTodaysSchedule();	// <<--------------
	DataLogging.logSample_Voltage(fReadVcc());	// Logs the Voltage into the log array 
}



void debugPrint(const char msg[], int Delay, bool newLine) {

	if (!_DEBUG_MSGS) return;
	if (newLine) Serial.println(msg);
	else Serial.print(msg);
	delay(Delay);
}

void debugPrint(char *msg, int Delay, bool newLine) {

	if (!_DEBUG_MSGS) return;
	if (newLine) Serial.println(msg);
	else Serial.print(msg);
	delay(Delay);
}

void debugPrint(int msg, int Delay, bool newLine) {
	if (!_DEBUG_MSGS) return;
	if (newLine) Serial.println(msg);
	else Serial.print(msg);
	delay(Delay);

}

/**
* Name: fReadVcc
* Description: This function uses the known internal reference value of the 328p (~1.1V) to calculate the VCC value which comes from a battery
* This was leveraged from a great tutorial found at https://code.google.com/p/tinkerit/wiki/SecretVoltmeter?pageId=110412607001051797704
* return: float value of the Vcc
*/
float fReadVcc() {
	/*
	* analogReference(INTERNAL);
	* V-battery ------- [ 10Mohm] ----- A0 ------- [ 2 M-ohm] ----- GND
	*/
	float V_battery;
	for (int i = 0; i < 4; i++) {
		int batVolt = analogRead(VOLTAGE_MEASURE_PIN);
		//Serial.print(analogRead(A0)); delay(100);
		//Serial.print(batVolt); delay(100);
		V_battery = batVolt * (6.6 / 1023.0); //Use the known iRef to calculate battery voltage
		//if (V_battery > 2.5) break;
		delay(10);
	}				   //Serial.print(V_battery); delay(100);
	return V_battery;
}

void DecideBatteryLevel(void) {
	float Vcc = fReadVcc();
	debugPrint(Vcc, 10, true);
	if (Vcc <= 5 && Vcc >= 3)
	{
		//Serial.println(Vcc); delay(10);
		Serial.print(F("Battery is Low, please replace it")); delay(10);
		TurnMotorON(100);
		MotorVoltLow = true;
		LEDStatus = !LEDStatus;
		digitalWrite(LED, LEDStatus);   // turn the LED on (HIGH is the voltage level)
		delay(500);
	}
	if (Vcc <= 3)
	{
		Serial.print(F("Battery Died, Vent will be OFF"));
		delay(1000);
	}
}

/////////////////////////Measure the temperature from Ds18B20///////////////////////
float ReadTemp() {
	byte i;
	byte present = 0;
	byte type_s;
	byte data[12];
	uint8_t addr[8];
	float Celsius, fahrenheit;

	if (!ds.search(addr)) {
		delay(10);
		if (!ds.search(addr)) {
			Serial.print("No more addresses."); delay(10);
			//Serial.print();
			ds.reset_search();
			delay(100);
			return -999.0;
		}
	}

	if (OneWire::crc8(addr, 7) != addr[7]) {
		Serial.print("CRC is not valid!"); delay(10);
		return -999.0;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1);        // start conversion, with parasite power on at the end
	delay(150);     // maybe 750ms is enough, maybe not
					// we might do a ds.depower() here, but the reset will take care of it.
	present = ds.reset();
	ds.select(addr);
	ds.write(0xBE);         // Read Scratch-pad
	for (i = 0; i < 9; i++) {           // we need 9 bytes
		data[i] = ds.read();
	}
	int16_t raw = (data[1] << 8) | data[0];
	if (type_s) {
		raw = raw << 3; // 9 bit resolution default
		if (data[7] == 0x10) {
			// "count remain" gives full 12 bit resolution
			raw = (raw & 0xFFF0) + 12 - data[6];
		}
	}
	else {
		byte cfg = (data[4] & 0x60);
		// at lower res, the low bits are undefined, so let's zero them
		if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
		else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
		else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
											  //// default is 12 bit resolution, 750 ms conversion time
	}
	Celsius = (float)raw / 16.0;
	fahrenheit = Celsius * 1.8 + 32.0;
	//Serial.print(" \n Temperature = ");
	//Serial.print(Celsius); delay(10);
	//Serial.print(" Celsius,  \n"); delay(10);

	return Celsius;
}
/////////////////////////// end of measuring Temp.///////////////////////////////////////

void pin_ISR() {
	InterruptedOnce = true;
	//buttonState = digitalRead(buttonPin);
	//digitalWrite(ledPin, buttonState);
}
