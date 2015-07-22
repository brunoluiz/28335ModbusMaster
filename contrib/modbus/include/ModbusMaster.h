#ifndef MODBUS_MASTER_H_
#define MODBUS_MASTER_H_

#include "ModbusDefinitions.h"
#include "ModbusData.h"
#include "Serial.h"
#include "Timer.h"
#include "Crc.h"

//typedef struct ModbusMaster ModbusMaster;

#include "ModbusRequestHandler.h"
#include "ModbusSettings.h"

struct ModbusMaster {
	ModbusMasterState state;

	ModbusData dataRequest;
	ModbusData dataResponse;

	ModbusRequestHandler requestHandler;

	Serial serial;
	Timer timer;

	Uint16 timeoutCounter;
	Uint16 successfulRequests;
	bool timeout;
	bool requestReady;
	bool requestProcessed;

	void (*loopStates)(ModbusMaster *self);
	void (*create)(ModbusMaster *self);
	void (*start)(ModbusMaster *self);
	void (*request)(ModbusMaster *self);
	void (*receive)(ModbusMaster *self);
	void (*process)(ModbusMaster *self);
	void (*destroy)(ModbusMaster *self);
};

void master_loopStates(ModbusMaster *self);
inline void master_create(ModbusMaster *self);
inline void master_start(ModbusMaster *self);
inline void master_receive(ModbusMaster *self);
inline void master_process(ModbusMaster *self);
inline void master_destroy(ModbusMaster *self);
ModbusMaster construct_ModbusMaster();

extern ModbusMaster mb;

#endif
