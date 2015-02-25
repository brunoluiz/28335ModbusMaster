#include "ModbusMaster.h"
#include "Log.h"

#if DEBUG_UTILS_PROFILING
#include "Profiling.h"
ProfilingTool profiling;
#endif

void master_loopStates(ModbusMaster *self){
	MB_MASTER_DEBUG();
	switch (self->state) {
	case MB_CREATE:
		MB_MASTER_DEBUG("State: MB_MASTER_CREATE\n");
		self->create(self);
		break;
	case MB_START:
		MB_MASTER_DEBUG("State: MB_MASTER_START\n");
		self->start(self);
		break;
	case MB_REQUEST:
		MB_MASTER_DEBUG("State: MB_MASTER_REQUEST\n");
		self->request(self);
		break;
	case MB_RECEIVE:
		MB_MASTER_DEBUG("State: MB_MASTER_RECEIVE\n");
		self->receive(self);
		break;
	case MB_PROCESS:
		MB_MASTER_DEBUG("State: MB_MASTER_PROCESS\n");
		self->process(self);
		break;
	case MB_DESTROY:
		MB_MASTER_DEBUG("State: MB_MASTER_DESTROY\n");
		self->destroy(self);
		break;
	}
}

void master_create(ModbusMaster *self){
	MB_MASTER_DEBUG();

	// Configure Serial Port A
	self->serial.baudrate = SERIAL_BAUDRATE;
	self->serial.parityType = SERIAL_PARITY;
	self->serial.bitsNumber = SERIAL_BITS_NUMBER;
	self->serial.init(&self->serial);

	self->timer.init(&self->timer, MB_REQ_TIMEOUT);

#if DEBUG_UTILS_PROFILING
	profiling = construct_ProfilingTool();
#endif

	self->state = MB_START;
}

void master_start(ModbusMaster *self){
	MB_MASTER_DEBUG();

	self->timer.resetTimer();
	
	self->dataRequest.clear(&self->dataRequest);
	self->dataResponse.clear(&self->dataResponse);

	self->serial.clear();
	self->timeout = false;

	self->state = MB_REQUEST;
}

void master_request(ModbusMaster *self){
	Uint16 * transmitString;
	Uint16 registerContents[2];


	// Implementing a demo request
	registerContents[0] = 0x0001;
	registerContents[1] = 0xFFFF;

	mb.requestHandler.slaveAddress = 0x01;
	mb.requestHandler.functionCode = MB_FUNC_WRITE_NREGISTERS;
	mb.requestHandler.firstAddr	   = 0x02;
	mb.requestHandler.totalData    = 2;
	mb.requestHandler.content      = registerContents;
	mb.requestHandler.generate(self);

	// Wait until the code signals that the request is ready to transmit
	while(self->requestHandler.isReady == false ) { }
	// Reset request ready signal
	self->requestHandler.isReady = false;

	// Prepares data request string
//	self->dataRequest.size = MB_SIZE_COMMON_DATA + self->dataRequest.contentIdx;
//	sizeWithoutCRC = self->dataRequest.size - 2;
//	transmitStringWithoutCRC = self->dataResponse.getTransmitStringWithoutCRC(&self->dataRequest);
//	self->dataRequest.crc = generateCrc( transmitStringWithoutCRC, sizeWithoutCRC, true );


	transmitString = self->dataResponse.getTransmitString(&self->dataRequest);
	self->timer.start();
#if DEBUG_UTILS_PROFILING
	profiling.start(&profiling);
#endif
	self->serial.transmitData(transmitString, self->dataRequest.size);

	MB_MASTER_DEBUG();

	self->state = MB_RECEIVE;
}


void master_receive(ModbusMaster *self){
	Uint16 fifoWaitBuffer = 0;

	MB_MASTER_DEBUG();

	self->requestProcessed = false;

	if (self->dataRequest.functionCode == MB_FUNC_READ_HOLDINGREGISTERS) {
		fifoWaitBuffer = MB_SIZE_COMMON_DATA;
		fifoWaitBuffer += self->dataRequest.content[3] * 2;
		fifoWaitBuffer += 1;
	}
	else {
		fifoWaitBuffer = MB_SIZE_RESP_WRITE;
	}

	while (
		self->serial.rxBufferStatus() < fifoWaitBuffer
		&& ( self->serial.getRxError() == false )
		&& ( self->timer.expiredTimer(&self->timer) == false )
	){	}

	self->timer.stop();
#if DEBUG_UTILS_PROFILING
	profiling.stop(&profiling);
#endif

	self->state = MB_START;

	// If there is any error on Reception, it will go to the START state
	if (self->serial.getRxError() == true || self->timeout == true){
		self->state = MB_START;
	} else {
//		Uncomment below if you don't need to process any data
//		self->successfulRequests++;
//		self->state = MB_MASTER_START;
		self->state = MB_PROCESS;
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
	self->requestProcessed = true;

	self->state = MB_START;
}

void master_destroy(ModbusMaster *self){
	MB_MASTER_DEBUG();
}

ModbusMaster construct_ModbusMaster(){
	ModbusMaster modbusMaster;

	MB_MASTER_DEBUG();

	modbusMaster.state = MB_CREATE;
	modbusMaster.dataRequest = construct_ModbusData();
	modbusMaster.dataResponse = construct_ModbusData();
	modbusMaster.requestHandler = construct_ModbusRequestHandler();
	modbusMaster.serial = construct_Serial();
	modbusMaster.timer = construct_Timer();

	modbusMaster.timeoutCounter = 0;
	modbusMaster.successfulRequests = 0;
	modbusMaster.requestReady = false;
	modbusMaster.requestProcessed = false;

	modbusMaster.loopStates = master_loopStates;
	modbusMaster.create = master_create;
	modbusMaster.start = master_start;
	modbusMaster.request = master_request;
	modbusMaster.receive = master_receive;
	modbusMaster.process = master_process;
	modbusMaster.destroy = master_destroy;

	return modbusMaster;
}
