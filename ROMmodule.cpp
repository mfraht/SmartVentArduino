//#include "energyarduino.h"
#include "ROMmodule.h"

cClockROM_Module::cClockROM_Module() {
	//checkROMcode();

}

void cClockROM_Module::init()
{
	//uint8_t d = 3;
	//digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	////delay(2000);
	//byte d[800];
	/*for (int i = 0; i < sizeof(d); i++) {
		d[i] = 255-(i%255);
	}
	writeToROM(0, (byte*)d, sizeof(d));*/
	//digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	//rtc.eeprom_write(1, (uint8_t)9);
	//rtc.eeprom_write(1, (uint8_t)5); //d += 3;
	//readFromROM(0, (byte*)(d), sizeof(d)); 
	///*d = rtc.eeprom_read(0);*/
	//Serial.print("init d= "); Serial.println(d);
	//for (int i = 0; i < sizeof(d); i++) {
	//	//if (i == 5) i = 20;
	//	//Serial.print(i); Serial.print(": "); Serial.println((uint8_t)rtc.eeprom_read(i));
	//	Serial.print(i); Serial.print(": "); Serial.println(d[i]);
	//	delay(5);
	//}

	// checkROMcode();
}

/////////////////////////  ROM Functions  ////////////////////////////////////////////

// Write the given data at the given position into the ROM with the given length. Return true if successful
bool cClockROM_Module::writeToROM(uint16_t position, byte * data, uint16_t length) {
	//rtc.eeprom_write(0, data, 1); return;
	// Serial.print("writeToROM position= "); Serial.println(position);
	// Serial.print("writeToROM length= "); Serial.println(length);
	/*Serial.print("writeToROM data= "); Serial.println(*data);*/
	digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	delay(10);	//  Delay before reading
	bool r = true;
	//uint16_t inc = 0;
	//uint8_t s;
	//for (uint16_t inc = 0; length > inc; inc+= 255) {
	//	//inc = i * 256;
	//	s = length - inc < 255 ? length - inc : 255;
	//	Serial.print("writeToROM s= "); Serial.println(s); delay(10);
	//	/*Serial.print("writeToROM position + inc= "); Serial.println(position + inc);
	//	Serial.print("writeToROM inc= "); Serial.println(inc);
	//	Serial.print("writeToROM *(data + inc)= "); Serial.println(*(data + inc));*/
	//	//uint8_t d = 66;
	//	//r = rtc.eeprom_write(0, data,1);
	//	r = rtc.eeprom_write(position + inc, (void*)(data + inc), s); delay(100);
	//	if (!r) {
	//		Serial.print("Error in writing. inc= "); Serial.println(inc);			
	//		break;
	//	}
	//}
	for (uint16_t i = 0; i < length; i++) {
		//inc = i * 256;
		/*Serial.print("writeToROM position + i= "); Serial.print(position + i);
		Serial.print(", writeToROM *(data + "); Serial.print(i); Serial.print(") = ");
		Serial.print((const uint8_t)*(data + i)); 
		Serial.print(", sizeof= "); Serial.println(sizeof((const uint8_t)*(data + i)));*/
		//rtc.eeprom_write(1, (uint8_t)8); delay(10);
		r = rtc.eeprom_write(position + i, (unsigned char)(*(data + i))); delay(5);
		if (!r) {
			Serial.print(F("Error in writing. i= ")); Serial.println(i);
			break;
		}
	}
	delay(10);
	digitalWrite(ClockPowerPin, LOW); // Power off the clock
	return r;
}
template <typename TW> bool cClockROM_Module::writeToROM(uint16_t position, const TW data) {
	digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	delay(10);	//  Delay before reading
	/*bool r = rtc.eeprom_write(position, (void *)&data, sizeof(TW));*/
	bool r = rtc.eeprom_write(position, data);
	digitalWrite(ClockPowerPin, LOW); // Power off the clock
	return r;
}
bool cClockROM_Module::writeToROM(unsigned int position, int16_t data) {
	digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	delay(10);	//  Delay before reading
	bool r = rtc.eeprom_write(position, data);
	digitalWrite(ClockPowerPin, LOW); // Power off the clock
	return r;
}

// Read the from the given position into the result for the given length. 
void cClockROM_Module::readFromROM(unsigned int position, byte * result, uint16_t length) {
	digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	delay(10);	//  Delay before reading
	uint8_t s;
	/*Serial.print("readFromROM length= "); Serial.println(length);
	Serial.print("readFromROM position= "); Serial.println(position);*/
	//for (uint16_t inc = 0; length > inc; inc += 256) {
	//	//inc = i * 256;
	//	s = length - inc < 256 ? length - inc : 256;
	//	rtc.eeprom_read(position + inc, (byte*)(result + inc), s); delay(100);
	//	/*Serial.print("readFromROM result + inc= "); Serial.println(*(result + inc));
	//	Serial.print("readFromROM s= "); Serial.println(s);			*/
	//}
	for (uint16_t i = 0; i < length; i++) {
		result[i] = rtc.eeprom_read(position + i); //delay(10);
		/*Serial.print("readFromROM result + inc= "); Serial.println(*(result + inc));
		Serial.print("readFromROM s= "); Serial.println(s);			*/
	}
	digitalWrite(ClockPowerPin, LOW); // Power off the clock
}


/////////////////////////  Clock Functions  ////////////////////////////////////////////

// Sets the Clock module time
//  setModuleClock(byte second, byte minute, byte hour, byte dayOfWeek<1-7 start with Sunday>, byte dayOfMonth, byte month, byte year<e.g. 18>)
bool cClockROM_Module::setModuleClock(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year) {
	digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	delay(140);	//  Delay before reading
	//  RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
	rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
	digitalWrite(ClockPowerPin, LOW); // Power off the clock
	return true;
}

//  setModuleClock(int second, int minute, int hour, int dayOfWeek<1-7 start with Sunday>, int dayOfMonth, int month, int year<e.g.2018>)
bool cClockROM_Module::setModuleClock(int second, int minute, int hour, int dayOfWeek, int dayOfMonth, int month, int year) {
	digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	delay(140);	//  Delay before reading
				//  RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
	rtc.set((byte)second, (byte)minute, (byte)hour, (byte)dayOfWeek, (byte)dayOfMonth, (byte)month, (byte)(year-2000));
	digitalWrite(ClockPowerPin, LOW); // Power off the clock
	return true;
}

bool cClockROM_Module::setModuleClock(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek,\
										uint8_t dayOfMonth, uint8_t month, uint16_t year) {
	digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	delay(140);	//  Delay before reading
				//  RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
	rtc.set((uint8_t)second, (uint8_t)minute, (uint8_t)hour, (uint8_t)dayOfWeek, (uint8_t)dayOfMonth, (uint8_t)month, (uint8_t)(year - 2000));
	digitalWrite(ClockPowerPin, LOW); // Power off the clock
	return true;
}

//  setModuleClock(timeDate * td)
bool cClockROM_Module::setModuleClock(timeDate * td) {
	digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	delay(140);	//  Delay before reading
				//  RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
	rtc.set((byte)(td->Current_Second), (byte)(td->Current_Minute), (byte)(td->Current_Hour), \
		(byte)(td->Current_WeekDay), (byte)(td->Current_DayOfMonth), (byte)(td->Current_Month), (byte)(td->Current_Year- 2000));
	digitalWrite(ClockPowerPin, LOW); // Power off the clock
	return true;
}


void cClockROM_Module::syncClockFromModule(timeDate * td) {	// Sync the Arduino clock with the Clock module
	digitalWrite(ClockPowerPin, HIGH); // Power on the clock
	delay(140);	//  Delay before reading
	rtc.refresh();
	td->Current_WeekDay = rtc.dayOfWeek();
	td->Current_DayOfMonth = rtc.day();
	td->Current_Month = rtc.month();
	td->Current_Year = rtc.year() + 2000;
	td->Current_Hour = rtc.hour();
	td->Current_Minute = rtc.minute();
	td->Current_Second = rtc.second();
	digitalWrite(ClockPowerPin, LOW); // Power off the clock
}
