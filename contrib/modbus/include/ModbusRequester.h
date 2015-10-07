#ifndef MODBUS_REQUESTHANDLER_H_
#define MODBUS_REQUESTHANDLER_H_

#include "ModbusDefinitions.h"
#include "ModbusSettings.h"

typedef struct ModbusRequester ModbusRequester;
typedef struct ModbusMaster ModbusMaster;

struct ModbusRequester {
	Uint16 slaveAddress;
	ModbusFunctionCode functionCode;
	Uint16 addr;
	Uint16 totalData;

	// Write functions special
	Uint16 * content;

	bool isReady;

	void (*setContent)(ModbusMaster *master, Uint16 * content, Uint16 length);
	void (*generate)(ModbusMaster *master);
	void (*save)(ModbusMaster *save);
};

inline void requester_setContent(ModbusMaster *master, Uint16 * content, Uint16 length);
inline void requester_generate(ModbusMaster *master);
inline void requester_save(ModbusMaster *master);
ModbusRequester construct_ModbusRequestHandler();

#endif
