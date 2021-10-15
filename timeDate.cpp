#include "timeDate.h"
/**
* Name: printTime
* Description:  Print the Current date and time in Arduino
* @param timeDate * input: none
* output :
*/
void printTime(timeDate *t)
{
	Serial.print(t->Current_WeekDay); Serial.print(F(","));
	Serial.print(t->Current_Year); Serial.print(F("/"));	delay(10);
	Serial.print(t->Current_Month); Serial.print(F("/"));
	Serial.print(t->Current_DayOfMonth); Serial.print(F(","));	 delay(10);
	Serial.print(t->Current_Hour); Serial.print(F(":"));
	Serial.print(t->Current_Minute); Serial.print(F(":"));
	Serial.print(t->Current_Second);	delay(20);
}

void copyTimeDate(struct timeDate* source, struct timeDate* destination) {
	/*destination->Current_DayOfMonth = source->Current_DayOfMonth;
	destination->Current_Hour = source->Current_Hour;
	destination->Current_Minute = source->Current_Minute;
	destination ->Current_Month = source ->Current_Month ;
	destination ->Current_Second = source ->Current_Second ;
	destination ->Current_WeekDay = source ->Current_WeekDay ;
	destination ->Current_Year = source ->Current_Year ;*/
	memcpy(destination, source, sizeof(*source));
}