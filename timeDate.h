#pragma once
#ifndef TIMEDATE
#define TIMEDATE

#include "arduino.h"
struct timeDate
{
	uint8_t Current_Hour = 11; uint8_t Current_Minute = 57; uint8_t Current_Second = 0;
	uint8_t Current_WeekDay = 1; uint8_t Current_DayOfMonth = 1; uint8_t Current_Month = 1; uint16_t Current_Year = 2017;
};

/**
* Name: printcurrenttime
* Description:  Print the Current date and time in Arduino
* @param char * input: none
* output :
*/
void printTime(timeDate *t);

void copyTimeDate(struct timeDate* source, struct timeDate* destination);

#endif // !TIMEDATE
