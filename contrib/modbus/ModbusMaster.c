#include "ModbusMaster.h"
#include "ModbusData.h"
#include "ModbusDefinitions.h"
#include "ModbusSettings.h"
#include "Log.h"

void master_loopStates(ModbusMaster *self){
	MB_SLAVE_DEBUG();
	switch (self->state) {
	case MB_MASTER_CREATE:
		MB_SLAVE_DEBUG("State: MB_MASTER_CREATE\n");
		self->create(self);
		break;
	case MB_MASTER_START:
		MB_SLAVE_DEBUG("State: MB_MASTER_START\n");
		self->start(self);
		break;
	case MB_MASTER_REQUEST:
		MB_SLAVE_DEBUG("State: MB_MASTER_REQUEST\n");
		self->request(self);
		break;
	case MB_MASTER_RECEIVE:
		MB_SLAVE_DEBUG("State: MB_MASTER_RECEIVE\n");
		self->receive(self);
		break;
	case MB_MASTER_PROCESS:
		MB_SLAVE_DEBUG("State: MB_MASTER_PROCESS\n");
		self->process(self);
		break;
	case MB_MASTER_DESTROY:
		MB_SLAVE_DEBUG("State: MB_MASTER_DESTROY\n");
		self->destroy(self);
		break;
	}
}

void master_create(ModbusMaster *self){
	MB_SLAVE_DEBUG();

	self->serial.baudrate = SERIAL_BAUDRATE;
	self->serial.parityType = SERIAL_PARITY_NONE;
	self->serial.bitsNumber = SERIAL_BITS_NUMBER;
	self->serial.init(&self->serial);

	self->serial.setSerialTxEnabled(true);
	self->serial.setSerialRxEnabled(true);

	self->timer.init(&self->timer, MB_REQ_TIMEOUT);

	self->state = MB_MASTER_START;
}

void master_start(ModbusMaster *self){
	MB_SLAVE_DEBUG();

	self->timer.resetTimer();
	
	self->dataRequest.clear(&self->dataRequest);
	self->dataResponse.clear(&self->dataResponse);

	self->serial.clear();
	self->timeout = false;

	self->state = MB_MASTER_REQUEST;
}

void master_request(ModbusMaster *self){
	Uint16 * transmitStringWithoutCRC;
	Uint16 * transmitString;
	Uint16 sizeWithoutCRC;

	static Uint16 requestAddr = 0;

	// TODO: Implement what you want to request here

	// 11 03 006B 0003 7687
	self->dataRequest.slaveAddress = 0x11;
	self->dataRequest.functionCode = MB_FUNC_READ_HOLDINGREGISTERS;
	self->dataRequest.content[self->dataRequest.contentIdx++] = (requestAddr & 0xFF) >> 8;
	self->dataRequest.content[self->dataRequest.contentIdx++] = requestAddr & 0xFF;
	self->dataRequest.content[self->dataRequest.contentIdx++] = 0x00;
	self->dataRequest.content[self->dataRequest.contentIdx++] = 0x03;

	requestAddr++;

	self->dataRequest.size = MB_SIZE_COMMON_DATA + self->dataRequest.contentIdx;

	sizeWithoutCRC = self->dataRequest.size - 2;
	transmitStringWithoutCRC = self->dataResponse.getTransmitStringWithoutCRC(&self->dataRequest);
	self->dataRequest.crc = generateCrc( transmitStringWithoutCRC, sizeWithoutCRC );

	transmitString = self->dataResponse.getTransmitString(&self->dataRequest);
	self->serial.transmitData(transmitString, self->dataRequest.size);

	MB_SLAVE_DEBUG();

	self->state = MB_MASTER_RECEIVE;
}


void master_receive(ModbusMaster *self){
	Uint16 fifoWaitBuffer = 0;

	MB_SLAVE_DEBUG();

	if (self->dataRequest.functionCode == MB_FUNC_READ_HOLDINGREGISTERS) {
		fifoWaitBuffer = MB_SIZE_COMMON_DATA;
		fifoWaitBuffer += self->dataRequest.content[3] * 2;
		fifoWaitBuffer += 1;
	}
	else {
		fifoWaitBuffer = MB_SIZE_RESP_WRITE;
	}

	self->timer.start();

	while (
		self->serial.rxBufferStatus() < fifoWaitBuffer
		&& ( self->serial.getRxError() == false )
		&& ( self->timeout == false )
	){
		if ( self->timer.expiredTimer(&self->timer) ) {
			self->timeout = true;
			self->timeoutCounter++;
		}
	}

	self->timer.stop();

	// If there is any error on Reception, it will go to the START state
	if (self->serial.getRxError() == true || self->timeout == true){
		self->state = MB_MASTER_START;
	} else {
		self->state = MB_MASTER_PROCESS;
	}
}

void master_process(ModbusMaster *self){
	Uint16 contentSize;
	Uint16 contentIterator;

	self->dataResponse.slaveAddress = self->serial.getRxBufferedWord();
	self->dataResponse.functionCode = self->serial.getRxBufferedWord();

	if (self->dataRequest.functionCode == MB_FUNC_READ_HOLDINGREGISTERS) {
		contentSize = self->dataRequest.content[3] * 2 + 1;
	}
	else {
		contentSize = MB_SIZE_CONTENT_NORMAL;
	}

	for (contentIterator = 0; contentIterator < contentSize; contentIterator++){
		self->dataResponse.content[self->dataResponse.contentIdx++] = self->serial.getRxBufferedWord();
	}

	self->dataResponse.crc = (self->serial.getRxBufferedWord() << 8) |
			self->serial.getRxBufferedWord();

	self->successfulRequests++;

	self->state = MB_MASTER_START;
}

void master_destroy(ModbusMaster *self){
	MB_SLAVE_DEBUG();
}

ModbusMaster construct_ModbusMaster(){
	ModbusMaster modbusMaster;

	MB_SLAVE_DEBUG();

	modbusMaster.state = MB_MASTER_CREATE;
	modbusMaster.dataRequest = construct_ModbusData();
	modbusMaster.dataResponse = construct_ModbusData();
	modbusMaster.serial = construct_Serial();
	modbusMaster.timer = construct_Timer();

	modbusMaster.timeoutCounter = 0;
	modbusMaster.successfulRequests = 0;

	modbusMaster.loopStates = master_loopStates;
	modbusMaster.create = master_create;
	modbusMaster.start = master_start;
	modbusMaster.request = master_request;
	modbusMaster.receive = master_receive;
	modbusMaster.process = master_process;
	modbusMaster.destroy = master_destroy;

	return modbusMaster;
}
