#include "ModbusMaster.h"
#include "ModbusSettings.h"
#include "Log.h"

#if DEBUG_UTILS_PROFILING
#include "Profiling.h"
ProfilingTool profiling;
#endif

Uint64 ticks = 0;

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
	case MB_WAIT:
		self->wait(self);
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
	
	self->dataRequest.clear(&self->dataRequest);
	self->dataResponse.clear(&self->dataResponse);

	self->serial.clear();
	self->timeout = false;

	self->timer.resetTimer();
	self->timer.setTimerReloadPeriod(&self->timer, MB_REQ_INTERVAL);
	self->timer.start();

	self->state = MB_WAIT;
}

void master_wait(ModbusMaster *self) {
	if (self->timer.expiredTimer(&self->timer)) {
		self->timer.stop();
		self->state = MB_REQUEST;
	}
}

void master_request(ModbusMaster *self){
	Uint16 * transmitString;

	// Wait until the code signals that the request is ready to transmit
	while (self->requester.isReady == false ) { }
	// Reset request ready signal
	self->requester.isReady = false;

	transmitString = self->dataResponse.getTransmitString(&self->dataRequest);
#if DEBUG_UTILS_PROFILING
	profiling.start(&profiling);
#endif
	self->serial.transmitData(transmitString, self->dataRequest.size);

	MB_MASTER_DEBUG();

	self->state = MB_RECEIVE;
}

void master_receive(ModbusMaster *self){
	self->timer.resetTimer();
	self->timer.setTimerReloadPeriod(&self->timer, MB_REQ_TIMEOUT);
	self->timer.start();

	self->requestProcessed = false;

	// Wait to have some date at the RX Buffer
	// If removed it can give errors on address and function code receiving
	while (self->serial.rxBufferStatus() < 2) { ; }

	// Get basic data from response
	self->dataResponse.slaveAddress = self->serial.getRxBufferedWord();
	self->dataResponse.functionCode = self->serial.getRxBufferedWord();

	// Prepare the buffer size
	if (self->dataRequest.functionCode == MB_FUNC_READ_HOLDINGREGISTERS) {
		self->serial.fifoWaitBuffer = MB_SIZE_COMMON_DATA_WITHOUTCRC;
		self->serial.fifoWaitBuffer += self->dataRequest.content[3] * 2;
		self->serial.fifoWaitBuffer += 1;
	}
	else {
		self->serial.fifoWaitBuffer = MB_SIZE_RESP_WRITE;
	}

	// Receive the data contents, based on the buffer size
	while ( ( self->serial.fifoWaitBuffer > 0 )
		&& ( self->serial.getRxError() == false )
		&& ( self->timer.expiredTimer(&self->timer) == false )
	) {
		if(self->serial.rxBufferStatus() > 0) {
			self->dataResponse.content[self->dataResponse.contentIdx++] = self->serial.getRxBufferedWord();
			self->serial.fifoWaitBuffer--;
		}
	}

	// Receive the CRC
	self->dataResponse.crc = (self->serial.getRxBufferedWord() << 8) | self->serial.getRxBufferedWord();

	self->timer.stop();
#if DEBUG_UTILS_PROFILING
	profiling.stop(&profiling);
#endif

	// Jump to START if there is any problem with the basic info
	if (self->dataResponse.slaveAddress != self->dataRequest.slaveAddress ||
			self->dataResponse.functionCode != self->dataRequest.functionCode ) {
		self->state = MB_START;
		return ;
	}

	// If there is any error on Reception, it will go to the START state
	if (self->serial.getRxError() == true || self->timer.expiredTimer(&self->timer)){
		self->state = MB_START;
	} else {
		self->state = MB_PROCESS;
	}
}

void master_process (ModbusMaster *self){
	self->requester.save(self);

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
	modbusMaster.requester = construct_ModbusRequestHandler();
	modbusMaster.serial = construct_Serial();
	modbusMaster.timer = construct_Timer();

	modbusMaster.timeoutCounter = 0;
	modbusMaster.successfulRequests = 0;
	modbusMaster.requestReady = false;
	modbusMaster.requestProcessed = false;

#if MB_COILS_ENABLED
	modbusMaster.coils = construct_ModbusCoilsMap();
#endif
#if MB_INPUTS_ENABLED
	modbusMaster.inputs = construct_ModbusInputsMap();
#endif
#if MB_HOLDING_REGISTERS_ENABLED
	modbusMaster.holdingRegisters = construct_ModbusHoldingRegistersMap();
#endif
#if MB_INPUT_REGISTERS_ENABLED
	modbusMaster.inputRegisters = construct_ModbusInputRegistersMap();
#endif

	modbusMaster.loopStates = master_loopStates;
	modbusMaster.create = master_create;
	modbusMaster.start = master_start;
	modbusMaster.wait = master_wait;
	modbusMaster.request = master_request;
	modbusMaster.receive = master_receive;
	modbusMaster.process = master_process;
	modbusMaster.destroy = master_destroy;

	return modbusMaster;
}
