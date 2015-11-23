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
	void (*save)(ModbusMaster *master);
	int (*request)(ModbusMaster *mb, Uint16 slaveAddress, ModbusFunctionCode functionCode, Uint16 addr, Uint16 totalData);

	int (*readInputs)(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
	int (*readCoils)(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
	int (*readHolding)(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
	int (*readInputRegs)(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
	int (*forceCoils)(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
	int (*writeHolding)(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
};

inline void requester_generate(ModbusMaster *master);
inline void requester_save(ModbusMaster *master);
inline int  requester_request(ModbusMaster *mb, Uint16 slaveAddress, ModbusFunctionCode functionCode,
		Uint16 addr, Uint16 totalData);
inline int  requester_readInputs(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
inline int  requester_readCoils(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
inline int  requester_readHolding(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
inline int  requester_readInputRegs(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
inline int  requester_writeHolding(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
inline int  requester_forceCoils(ModbusMaster *mb, Uint16 slaveAddress, Uint16 addr, Uint16 totalData);
ModbusRequester construct_ModbusRequestHandler();

#endif
