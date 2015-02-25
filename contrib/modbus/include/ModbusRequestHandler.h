#ifndef MODBUS_REQUESTHANDLER_H_
#define MODBUS_REQUESTHANDLER_H_

#include "ModbusDefinitions.h"
#include "ModbusSettings.h"
#include MB_DATA_MAP

typedef struct ModbusRequestHandler ModbusRequestHandler;
typedef struct ModbusMaster ModbusMaster;

struct ModbusRequestHandler {
	Uint16 slaveAddress;
	ModbusFunctionCode functionCode;
	Uint16 firstAddr;
	Uint16 totalData;

	// Write functions special
	Uint16 * content;

	bool isReady;

	void (*setContent)(ModbusMaster *master, Uint16 * content, Uint16 length);
	void (*generate)(ModbusMaster *master);
};

inline void requestHandler_setContent(ModbusMaster *master, Uint16 * content, Uint16 length);
inline void requestHandler_generate(ModbusMaster *master);
ModbusRequestHandler construct_ModbusRequestHandler();

#endif
