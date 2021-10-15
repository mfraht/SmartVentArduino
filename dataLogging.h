// dataLogging.h
#include "timeDate.h"
#include "ROMmodule.h"
#define keepTestData true
#ifndef _DATALOGGING_h
#define _DATALOGGING_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define DATALOGGING_FORMAT_CODE 49				// Data log format code. Change this code to re-write the LOG ROM
#define TEMPERATURE_SAMPLING 60					// The Temperature sampling interval in minutes
#define VOLTAGE_SAMPLING (24 * 60)				// The voltage sampling interval in minutes \
 A Sample is the values at a given sample time (temp high and low are one sample as they are at the same sample time, \
Voltage is another sample even if at same time as it has label)
#define SAMPLE_LENGHT_BYTE 4		// The sample length in Bytes. Temp: 2bytes Low temp, 2 Bytes High temp. Voltage: 2 Bytes mark, 2 Bytes Volt*10
//#define FORMAT_TEMPERATURE_SAMPLE(th, tl) (((uint32_t)(th*10+2000) << 16) | (uint32_t)(tl*10+2000))	// Format the temperature sample into long with (th*10+200)<<16 | tl*10+200
//#define FORMAT_VOLTAGE_SAMPLE(v)	(((uint32_t)(v*10) << 8) | (uint32_t)40<<24 | (uint32_t)41)	// Format the voltage sample into long with (v)
#define HEADER_AREA_START_POSITION 20			// Header storage start position
#define HEADER_AREA_LENGTH (2 * 1024)			// Length of All Header area in Bytes (Main + Headers areas)
#define DATA_AREA_START_POSITION (HEADER_AREA_LENGTH) // Data batches storage start position
#define ROM_LENGTH 32768						// Total ROM length in Bytes
#define BATCH_LENGTH_DAYS 7						// Number of days in each log batch
//#define DATA_SAMPLE_LENGTH_BYTES sizeof(int16_t)// The number of bytes for each sample
#define HEADER_START_SIGNATURE "**%04d**"		// The data batch start signature
#define HEADER_START_SIGNATURE_LENGTH 8			// The data batch start signature length in Bytes
#define HEADER_END_SIGNATURE "##%04d##"			// The data batch end signature
#define HEADER_END_SIGNATURE_LENGTH 8			// The data batch end signature length in Bytes
// Batch length given the Temperature and the voltage sample intervals
#define BATCH_LENGTH_BYTES(ts, vs) (uint16_t)(BATCH_LENGTH_DAYS * 24.0 * 60 * SAMPLE_LENGHT_BYTE * \
	(1.0 / (ts) + 1.0 / (vs)) + HEADER_START_SIGNATURE_LENGTH + HEADER_END_SIGNATURE_LENGTH)
#define BATCH_SAMPLE_LENGTH(ts, vs) (uint16_t)(BATCH_LENGTH_DAYS * 24.0 * 60 * (1.0 / (ts) + 1.0 / (vs)))	// Number of samples to store in One batch
#define LOG_ARRAY_SAMPLE_LENGTH(ts, vs) (uint16_t)(24.0 * 60 * (1.0 / (ts) + 1.0 / (vs)))	// Number of samples to store in the log array
#define MAX_NUMBER_BATCHES(ts, vs) ( ((ROM_LENGTH - HEADER_AREA_LENGTH) / BATCH_LENGTH_BYTES(ts, vs)) \
			< (HEADER_AREA_LENGTH / sizeof(LogBatchHeader)) ? \
			  (uint16_t)((ROM_LENGTH - HEADER_AREA_LENGTH) / BATCH_LENGTH_BYTES(ts, vs)) :	\
			  (uint16_t)(HEADER_AREA_LENGTH / sizeof(LogBatchHeader)))

struct ROM_MainHeader {
	uint8_t ROM_format_code = DATALOGGING_FORMAT_CODE;
	uint8_t headers_count = 0;	// Current number of headers. 
	uint8_t last_header_number = 0;	// Stores the number of the last created header
};

struct LogBatchHeader {	// Used to store the log batch headers
	uint8_t batch_number = 1;		// Starts from 1
	timeDate startDate;
	uint16_t temp_sample_interval_minutes = TEMPERATURE_SAMPLING;
	uint16_t volt_sample_interval_minutes = VOLTAGE_SAMPLING;
	bool shedule_enabled = true;	// Mark if the schedule was enabled at the beginning of this data sampling
	uint16_t num_of_samples = 0;
	uint16_t batch_start_position = DATA_AREA_START_POSITION;
	bool deleted = false;	// Used to mark the synchronized batches
	bool complete = false;	// Mark if the data samples are complete for this batch
};

class SampleClass {
	static const char voltage_start = '(';		// Start Character
	static const char voltage_end = ')';		// Start Character
	/*
	struct TemperatureSample { // Format the temperature sample into long with (th*10+200)<<16 | tl*10+200
		uint16_t temperature_high;	// High temperature * 10 + 200
		uint16_t temperature_low;	// Low temperature * 10 + 200
	};
	struct VoltageSample {
		char start = '(';		// Start Character
		uint16_t voltage = 0;	// Voltage * 10
		char end = ')';			// End character
	};*/
public:
	// Format the temperature sample into long with (th*10+200)<<16 | tl*10+200
	static uint32_t encode_temperature(float th, float tl) {
		return(((uint32_t)(th * 10 + 2000) << 16) | (uint32_t)(tl * 10 + 2000));	
	}
	// Format the voltage sample into long with (v*10)
	static uint32_t encode_voltage(float v) {
		return (((uint32_t)(v * 10) << 8) | (uint32_t)voltage_start << 24 | (uint32_t)voltage_end);	
	}
	// Decode temperature from long into 2 floats th, tl
	static void decode_temperature(float* results, uint32_t t) {
		uint16_t* p = (uint16_t *)(&t);
		results[0] = (1.0 * p[0] - 2000.0) / 10.0;
		results[1] = (1.0 * p[1] - 2000.0) / 10.0;
	}
	// Decode voltage from long into float
	static void decode_voltage(float* results, uint32_t v) {
		float p = (float)((v & 0x00ffff00) >> 8);
		results[0] = p / 10.0;		
	}
	// Check if this value is Voltage
	static bool isVoltage(uint32_t a) {
		char* p = (char *)(&a);
		//Serial.print(" ,p0"); Serial.print(p[0]);
		return (p[3] == voltage_start) && (p[0] == voltage_end);
	}
};

class DataLoggingClass
{
protected:
	bool initialized = false;	// indicate if the class initialized or not
	ROM_MainHeader romMainHeader;
	LogBatchHeader  currentLogBatchHeader;
	//uint32_t logArray[10] = {0};
	uint32_t logArray[LOG_ARRAY_SAMPLE_LENGTH(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING)] = { 0 };
	uint16_t logArrayCount = 0;
	bool * pDisableSchedule;

	cClockROM_Module *pCR;
	void checkROMcode(); // Checks ROM code and overwrite if not same code
	void writeHeaderToROM(LogBatchHeader *plbh, uint16_t position = 0);	// Writes the given header in Rom. If no position is given, it will be calculated
	void writeROM_MainHeaderToROM();	// Writes the Main header into Rom 
	void readROM_MainHeaderFromROM();	// Reads a the Main header from ROM
	LogBatchHeader createBatch(uint16_t number = 0);	// Create a new Batch and its header into ROM
	void endBatch(LogBatchHeader *pbh);		// End the batch and write the sample count in the header and write the header
	uint16_t  writeSamples(byte *samples, uint16_t count);	// Write the given samples into current batch. Adds a new batch if needed
	void addSamplesToBatch(byte * samples, LogBatchHeader *pbh, uint16_t length_bytes);	// Writes the given samples to the ROM and update the header
	uint16_t calculateHeaderPosition(uint8_t headerNumber); // Calculates the header position based on its number. \
															 Cycle when the number > MAX_NUMBER_BATCHES
	uint16_t calculateDataPosition(uint16_t batchNumber);	// Calculates the data batch start position in bytes. Cycle
	//void printTimeDate(struct timeDate *td);
	void getHeaderNumber(LogBatchHeader *plbh, uint8_t number);// Reads the header "number" from ROM, into the given pointer
	void readSamplesFromBatch(byte * samples, LogBatchHeader *pbh, uint16_t start_sample_no = 1, \
		uint16_t count = 0);	// Reads (count) samples from the given batch beginning at start sample number

public:
	DataLoggingClass() {

	}

	void init(cClockROM_Module *pClock, bool * pDisSchedule);
	void logSample_Temperature(float th, float tl);	// Stores the given High and Low temperature sample into the log array 
	void logSample_Voltage(float v);	// Stores the given Voltage into the log array 
	uint8_t getHeadersCount();			// Reads number of headers stored
	uint8_t getLastHeadersNumber();		// Reads number of the last header

	void sendOutHeader(uint8_t number);	// Sends the header "number" on Serial port as JSON format
	void sendOutBatchData(uint8_t number);	// Sends the Samples in header "number" on Serial port as JSON format


};

//extern DataLoggingClass DataLogging;

#endif

