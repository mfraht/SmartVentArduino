// Data logging methods
// Stored each sample in 2 Bytes (int16)
// Samples are either Temperature (Low & High) or Voltage

#include "dataLogging.h"

void DataLoggingClass::init(cClockROM_Module *pClock, bool * pDisSchedule)
{
	if (initialized) return;
	pCR = pClock;
	pDisableSchedule = pDisSchedule;
	
	/*Serial.print("SizeOf logArray: ");	
	Serial.println(sizeof(logArray)); delay(10);*/
	checkROMcode();
	/*logSample_Temperature(0, 20.3);
	logSample_Temperature(10.6, 0);
	logSample_Temperature(10.6, 20.3);
	logSample_Temperature(-10.6, -20.3);
	logSample_Voltage(5.4);*/
	/*delay(10); delay(10);
	Serial.print("SizeOf logBatchHeader: ");	delay(10);
	Serial.println(sizeof(LogBatchHeader)); delay(10);
	Serial.print("SizeOf char: "); delay(10);
	Serial.println(sizeof(char));
	Serial.print("BATCH_LENGTH_BYTES: "); delay(10);
	Serial.println(BATCH_LENGTH_BYTES(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING)); delay(10);
	Serial.print("MAX_NUMBER_BATCHES: "); delay(10);
	Serial.println(MAX_NUMBER_BATCHES(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING)); delay(10);
	char s[40]; sprintf(s, HEADER_START_SIGNATURE, 5);
	Serial.print("HEADER_START_SIGNATURE: "); delay(10);
	Serial.println(s); delay(10);
	sprintf(s, HEADER_END_SIGNATURE, 15);
	Serial.print("HEADER_END_SIGNATURE: "); delay(10);
	Serial.println(String(s)); delay(10);
	Serial.print("HEADER_END_SIGNATURE Length: "); delay(10);
	Serial.println(sprintf(s, HEADER_END_SIGNATURE, 5)); delay(10);
	Serial.print("calculateDataPosition(10): "); delay(10);
	Serial.println(calculateDataPosition(10)); delay(10);
	Serial.print("LOG_ARRAY_SAMPLE_LENGTH(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING): "); delay(10);
	Serial.println(LOG_ARRAY_SAMPLE_LENGTH(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING)); delay(10);*/

	initialized = true;
}

// Checks ROM code and overwrite if not same code
void DataLoggingClass::checkROMcode() {
	// Check to make sure the ROM code is the same. Otherwise overwrite the ROM format
	readROM_MainHeaderFromROM();
	if (romMainHeader.ROM_format_code != DATALOGGING_FORMAT_CODE) {	// Overwrite
		romMainHeader.headers_count = 0;
		romMainHeader.ROM_format_code = DATALOGGING_FORMAT_CODE;
		// Make it all Zeros
		//for(uint16_t i = 0; i < sizeof(logArray); i++) logArray[i] = 0;
	//	for(uint16_t i=0; i < (HEADER_AREA_LENGTH - HEADER_AREA_START_POSITION); i+= sizeof(logArray))
	//		pCR->writeToROM((uint16_t)HEADER_AREA_START_POSITION+i, (byte*)logArray, sizeof(logArray));
		// Create and Store the 1st data header 
		currentLogBatchHeader = createBatch(1);		
		Serial.print(F("ROM_format_code: ")); Serial.println(romMainHeader.ROM_format_code);
		Serial.println(F("Testing ROM module...")); delay(10);
		// Writing test data
		for (int i = 0; i < 24*8; i++) {			
			if(i%24==0) logSample_Voltage(i/5.0+(i%10)/10.0);	// Stores the given Voltage into the log array 
			logSample_Temperature(i / 2.0, i / 2.0 + 10);	// Stores the given High and Low temperature sample into the log array 
		}		
		Serial.print(F("Headers count: ")); Serial.println(romMainHeader.headers_count);
		Serial.print(F("Last Header num_of_samples: ")); Serial.println(currentLogBatchHeader.num_of_samples); delay(10);
		if (currentLogBatchHeader.batch_number = !2 || currentLogBatchHeader.num_of_samples != 25) {
			Serial.println(F("Data test Error !!! "));
		}
		else {
			Serial.println(F("Data test OK :). "));
			if (!keepTestData) {
				romMainHeader.headers_count = 0;	// Delete test data
				currentLogBatchHeader = createBatch(1);
			}
		}
	}
	else {		
		//uint16_t position = (romMainHeader.headers_count - 1) * sizeof(LogBatchHeader) + HEADER_AREA_START_POSITION;
		getHeaderNumber(&currentLogBatchHeader, romMainHeader.last_header_number);
		if (currentLogBatchHeader.num_of_samples == 0) {			
			pCR->syncClockFromModule(&(currentLogBatchHeader.startDate)); 
			writeHeaderToROM(&currentLogBatchHeader);
			// Serial.println("Loaded headers. Reused last Header..."); delay(10);
		}
		else {
			currentLogBatchHeader = createBatch();
			// Serial.println("Loaded headers. Added new Header..."); delay(10);
		}		
		//Serial.print("currentLogBatchHeader.startDate: ");
		//printTime(&currentLogBatchHeader.startDate);
		//Serial.print("currentLogBatchHeader.batch_number: "); Serial.println(currentLogBatchHeader.batch_number); delay(10);
		// Serial.print("currentLogBatchHeader.batch_start_position: "); Serial.println(currentLogBatchHeader.batch_start_position); delay(10);
		//Serial.print("currentLogBatchHeader.num_of_samples: "); Serial.println(currentLogBatchHeader.num_of_samples); delay(10);
		/*Serial.print("currentLogBatchHeader.complete: "); Serial.println(currentLogBatchHeader.complete);
		Serial.print("currentLogBatchHeader.deleted: "); Serial.println(currentLogBatchHeader.deleted);
		Serial.print("currentLogBatchHeader.shedule_enabled: "); Serial.println(currentLogBatchHeader.shedule_enabled);
		Serial.print("currentLogBatchHeader.temp_sample_interval_minutes: "); Serial.println(currentLogBatchHeader.temp_sample_interval_minutes);
		Serial.print("currentLogBatchHeader.volt_sample_interval_minutes: "); Serial.println(currentLogBatchHeader.volt_sample_interval_minutes);
		*/
	}	
}

uint16_t DataLoggingClass::calculateHeaderPosition(uint8_t headerNumber) {
	uint16_t hs = MAX_NUMBER_BATCHES(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING);
	while(headerNumber > hs) headerNumber -= hs;	// e.g.: 1000 -> 1000 - 22*hs(44) -> 1000-968=32
	uint16_t position = (headerNumber - 1) * sizeof(LogBatchHeader) + HEADER_AREA_START_POSITION;
	/*Serial.print("calculateHeaderPosition position= "); Serial.println(position);
	Serial.print("calculateHeaderPosition headerNumber= "); Serial.println(headerNumber);*/
	return position;
}

uint16_t DataLoggingClass::calculateDataPosition(uint16_t batchNumber) {
	uint16_t hs = MAX_NUMBER_BATCHES(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING);
	while (batchNumber > hs) batchNumber -= hs;	// e.g.: 1000 -> 1000 - 22*hs(44) -> 1000-968=32
	uint16_t position = (batchNumber - 1) * \
		BATCH_LENGTH_BYTES(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING) \
		+ DATA_AREA_START_POSITION;
	return position;
}

void DataLoggingClass::writeHeaderToROM(LogBatchHeader *plbh, uint16_t position) {
	if(position == 0 ) position = calculateHeaderPosition(plbh->batch_number);
	pCR->writeToROM(position, (byte*)plbh, sizeof(LogBatchHeader));
}

void DataLoggingClass::getHeaderNumber(LogBatchHeader *plbh, uint8_t number) {
	uint16_t p = calculateHeaderPosition(number);
	//Serial.print("getHeaderNumber p= "); Serial.println(p); 
	//Serial.print("getHeaderNumber sizeof(LogBatchHeader)= "); Serial.println(sizeof(LogBatchHeader));
	//LogBatchHeader plbh ;
	pCR->readFromROM(p, (byte*)plbh, sizeof(LogBatchHeader));
	/*Serial.print("currentLogBatchHeader.batch_number: "); Serial.println(plbh->batch_number); delay(10);
	Serial.print("currentLogBatchHeader.batch_start_position: "); Serial.println(plbh->batch_start_position); delay(10);
	Serial.print("currentLogBatchHeader.num_of_samples: "); Serial.println(plbh->num_of_samples); delay(10);
	Serial.print("currentLogBatchHeader.complete: "); Serial.println(plbh.complete); delay(10);
	Serial.print("currentLogBatchHeader.deleted: "); Serial.println(plbh.deleted); delay(10);
	Serial.print("currentLogBatchHeader.shedule_enabled: "); Serial.println(plbh->shedule_enabled);
	Serial.print("currentLogBatchHeader.temp_sample_interval_minutes: "); Serial.println(plbh->temp_sample_interval_minutes);
	Serial.print("currentLogBatchHeader.volt_sample_interval_minutes: "); Serial.println(plbh->volt_sample_interval_minutes);
	Serial.print("currentLogBatchHeader.startDate: ");
	printTimeDate(&plbh->startDate); delay(20);*/
}

uint8_t DataLoggingClass::getHeadersCount() {
	return romMainHeader.headers_count;
}

uint8_t DataLoggingClass::getLastHeadersNumber() {
	return romMainHeader.last_header_number;
}

void DataLoggingClass::sendOutHeader(uint8_t number) {	// Sends the header "number" on Serial port as JSON format
	LogBatchHeader ch;
	getHeaderNumber(&ch, number);
	Serial.print(F("{\"")); Serial.print(ch.batch_number);
	Serial.print(F("\":{\"complete\":")); Serial.print(ch.complete);
	Serial.print(F(", \"deleted\":")); Serial.print(ch.deleted);
	Serial.print(F(", \"datalogging_format_code\":")); Serial.print(DATALOGGING_FORMAT_CODE);
	Serial.print(F(", \"samples_count\":")); Serial.print(ch.num_of_samples);
	Serial.print(F(", \"shedule_enabled\":")); Serial.print(ch.shedule_enabled);
	Serial.print(F(", \"temp_sampl_interval\":")); Serial.print(ch.temp_sample_interval_minutes);
	Serial.print(F(", \"volt_sampl_interval\":")); Serial.print(ch.volt_sample_interval_minutes);
	Serial.print(F(", \"start_Date\": \"")); printTime(&ch.startDate);
	Serial.print(F("\"}}"));
}

void DataLoggingClass::sendOutBatchData(uint8_t number) {	// Sends the Samples in header "number" on Serial port as JSON format
	LogBatchHeader ch;
	getHeaderNumber(&ch, number);
	Serial.print(F("{\"")); Serial.print(ch.batch_number);
	Serial.print(F("\":{\"data\":["));
	uint32_t sample; bool isVolt; float result[2];
	uint16_t maxSamples = BATCH_SAMPLE_LENGTH(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING);
	for (uint16_t n = 1; n <= ch.num_of_samples && n <= maxSamples; n++) {
		if( n!= 1 ) Serial.print(",");	// Middle commas
		readSamplesFromBatch((byte*)&sample, &ch, n, 1);
		Serial.print("{\"sn\":"); Serial.print(n);
		isVolt = SampleClass::isVoltage(sample);
		if (isVolt) {
			Serial.print(",\"V\":"); 
			SampleClass::decode_voltage(result, sample);
			Serial.print(result[0]);
		}
		else {
			Serial.print(",\"TH\":");
			SampleClass::decode_temperature(result, sample);
			Serial.print(result[0]);
			Serial.print(",\"TL\":");
			Serial.print(result[1]);
		}		
		Serial.print("}");
		//delay(10);
	}
	Serial.print(F("]}}"));
}

LogBatchHeader DataLoggingClass::createBatch(uint16_t number) {
	LogBatchHeader nh;
	nh.batch_number = number > 0 ? number : currentLogBatchHeader.batch_number + 1;
	nh.shedule_enabled = !*pDisableSchedule;
	pCR->syncClockFromModule(&(nh.startDate));
	nh.batch_start_position = calculateDataPosition(nh.batch_number);
	//(nh.batch_number - 1) * BATCH_LENGTH_BYTES(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING) + DATA_AREA_START_POSITION;
	writeHeaderToROM(&nh);
	char s[HEADER_START_SIGNATURE_LENGTH+1]; sprintf(s, HEADER_START_SIGNATURE, nh.batch_number);
	// Serial.print("createBatch s= "); Serial.println(s);
	pCR->writeToROM(nh.batch_start_position, (byte*)s, HEADER_START_SIGNATURE_LENGTH);
	romMainHeader.headers_count++;	// Increment the headers counter
	uint8_t m = MAX_NUMBER_BATCHES(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING);
	if (romMainHeader.headers_count > m)	romMainHeader.headers_count = m;	// Reached to the max count
	romMainHeader.last_header_number = nh.batch_number;
	writeROM_MainHeaderToROM();
	return nh;
}

void DataLoggingClass::endBatch(LogBatchHeader *pbh) {
	pbh->complete = true;
	writeHeaderToROM(pbh);	// To mark the stored samples count
	char s[HEADER_END_SIGNATURE_LENGTH + 1]; sprintf(s, HEADER_END_SIGNATURE, pbh->batch_number);
	// Serial.print("endBatch s= "); Serial.println(s);
	uint16_t p = pbh->batch_start_position + pbh->num_of_samples * SAMPLE_LENGHT_BYTE + \
				HEADER_START_SIGNATURE_LENGTH;
	pCR->writeToROM(p, (byte*)s, HEADER_END_SIGNATURE_LENGTH);	
	return ;
}

void DataLoggingClass::logSample_Temperature(float th, float tl) {
	logArray[logArrayCount] = SampleClass::encode_temperature(th, tl);
	uint16_t* p = (uint16_t *)(&logArray[logArrayCount]);
	// Serial.print("Temp log= "); Serial.print(p[1]); Serial.print(":"); Serial.println(p[0]);
	// (((uint32_t)(th*10+100) << 16) + (uint32_t)(tl*10+100))
	// Serial.print("F Temp= ");	Serial.print(th); Serial.print(":"); Serial.println(tl);
	logArrayCount++;
	if (logArrayCount * SAMPLE_LENGHT_BYTE == sizeof(logArray)) {
		//Serial.print("T logArrayCount= "); Serial.print(logArrayCount);
		//Serial.println(". Write Samples to ROM"); delay(10);
		logArrayCount = writeSamples((byte*)logArray, logArrayCount);
	}
}

void DataLoggingClass::logSample_Voltage(float v) {
	logArray[logArrayCount] = SampleClass::encode_voltage(v);
	// Serial.print("--Volt= ");	Serial.println(v);
	// Serial.print("--F Volt= ");	Serial.println(FORMAT_VOLTAGE_SAMPLE(v));
	//char* p = (char*)(&logArray[logArrayCount]);
	//uint16_t* pv = (uint16_t*)(p+1);
	//Serial.print("-R Volt= "); Serial.print(p[3]); Serial.print(*pv); Serial.println(p[0]);
	logArrayCount++;
	if (logArrayCount * SAMPLE_LENGHT_BYTE == sizeof(logArray)) {
		//Serial.print("V logArrayCount= "); Serial.print(logArrayCount); Serial.println(". Write Samples to ROM"); delay(10);
		logArrayCount = writeSamples((byte*)logArray, logArrayCount);
	}
}

uint16_t DataLoggingClass::writeSamples(byte *samples, uint16_t count) {
	uint16_t maxSamples = BATCH_SAMPLE_LENGTH(TEMPERATURE_SAMPLING, VOLTAGE_SAMPLING) ;
	uint16_t c = maxSamples - currentLogBatchHeader.num_of_samples > count ? \
		count : maxSamples - currentLogBatchHeader.num_of_samples;
	// Serial.print("writeSamples maxSamples= "); Serial.println(maxSamples);
	// Serial.print("writeSamples c= "); Serial.println(c);
	addSamplesToBatch((byte*)samples, &currentLogBatchHeader, c);
	if (currentLogBatchHeader.num_of_samples == maxSamples) {	// Batch is full
		endBatch(&currentLogBatchHeader);
		currentLogBatchHeader = createBatch();
	}	
	if ((count - c) > 0) {
		addSamplesToBatch((byte*)(samples + c), &currentLogBatchHeader, count - c);
	}
	if(currentLogBatchHeader.num_of_samples > 0)	writeHeaderToROM(&currentLogBatchHeader);
	return 0;
}

void DataLoggingClass::addSamplesToBatch(byte * samples, LogBatchHeader *pbh, uint16_t count) {
	uint16_t p = pbh->batch_start_position + pbh->num_of_samples * SAMPLE_LENGHT_BYTE + \
				HEADER_START_SIGNATURE_LENGTH;
	pCR->writeToROM(p, samples, count * SAMPLE_LENGHT_BYTE);
	pbh->num_of_samples += count;
}

void DataLoggingClass::readSamplesFromBatch(byte * samples, LogBatchHeader *pbh, \
											uint16_t start_sample_no, uint16_t count) {
	uint16_t p = pbh->batch_start_position + (start_sample_no - 1) * SAMPLE_LENGHT_BYTE + \
				HEADER_START_SIGNATURE_LENGTH;
	if (count == 0) count = pbh->num_of_samples - start_sample_no + 1;
	pCR->readFromROM(p, samples, count * SAMPLE_LENGHT_BYTE);	
}

void DataLoggingClass::writeROM_MainHeaderToROM() {
	pCR->writeToROM(0, (byte*)&romMainHeader, sizeof(romMainHeader));
}

void DataLoggingClass::readROM_MainHeaderFromROM() {
	pCR->readFromROM(0, (byte*)&romMainHeader, sizeof(romMainHeader));
}

// DataLoggingClass DataLogging;

