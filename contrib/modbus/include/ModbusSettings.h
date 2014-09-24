#ifndef MODBUSSETTINGS_H_
#define MODBUSSETTINGS_H_

#include "ModbusDefinitions.h"
#include "Serial.h"

// Modbus settings ========================================
#define MB_RTU_TCP			false			// Disable ID check and CRC check (TCP converter will do that)

// Modbus Master settings =================================
#define MB_REQ_TIMEOUT		1000000 // uSeconds

// Serial settings ========================================
#define SERIAL_BAUDRATE 	38400
#define SERIAL_PARITY 		SERIAL_PARITY_NONE
#define SERIAL_BITS_NUMBER 	8

// Debug settings =========================================
#define DEBUG_UTILS_PROFILING				true

// Log settings ===========================================
// Will be used at Log.h
#define DEBUG_LOG_ENABLED					false

#define MB_DATA_REQUEST_DEBUG_ENABLED		true
#define MB_DATA_RESPONSE_DEBUG_ENABLED		true
#define MB_DATA_HANDLER_DEBUG_ENABLED		true
#define MB_SLAVE_DEBUG_ENABLED				true
#define SERIAL_DEBUG_ENABLED				true
#define TIMER_DEBUG_ENABLED					true

#endif /* MODBUSSETTINGS_H_ */
