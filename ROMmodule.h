#pragma once
#include <uRTCLib.h>
//#define ClockPowerPin 6	//11 for old board, 6 for the new one
#include "timeDate.h"
#include "pins.h"

class cClockROM_Module {
private:
	uRTCLib rtc = uRTCLib(0x68, 0x57);
	
public:
	// Constructor
	cClockROM_Module();

	// Initialize
	void init();

	/////////////////////////  ROM Functions  ////////////////////////////////////////////	
	// Write the given data at the given position into the ROM with the given length. Return true if successful
	bool writeToROM(uint16_t position, byte * data, uint16_t length);
	template <typename TW> bool writeToROM(uint16_t position, const TW data);
	bool writeToROM(unsigned int position, int16_t data);
	//bool writeToROM(unsigned int position, uint16_t data);*/

	// Read the from the given position into the result for the given length. 
	void readFromROM(unsigned int position, byte * result, uint16_t length);


	/////////////////////////  Clock Functions  ////////////////////////////////////////////

	// Sets the Clock module time
	//  setModuleClock(byte second, byte minute, byte hour, byte dayOfWeek<1-7 start with Sunday>, byte dayOfMonth, byte month, byte year<e.g. 18>)
	bool setModuleClock(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
	bool setModuleClock(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint16_t year);
	
	//  setModuleClock(int second, int minute, int hour, int dayOfWeek<1-7 start with Sunday>, int dayOfMonth, int month, int year<e.g.2018>)
	bool setModuleClock(int second, int minute, int hour, int dayOfWeek, int dayOfMonth, int month, int year);

	//  setModuleClock(timeDate * td)
	bool setModuleClock(timeDate * td);

	void syncClockFromModule(timeDate * td);

};
