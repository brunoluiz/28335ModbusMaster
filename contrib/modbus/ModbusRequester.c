#include <ModbusRequester.h>
#include "ModbusDefinitions.h"
#include "ModbusMaster.h"
#include "Log.h"
#include "Crc.h"

void requester_setContent(ModbusMaster *master, Uint16 * content, Uint16 length) {
	;
}
void requester_generate(ModbusMaster *master) {
	ModbusRequester requester = master->requester;
	Uint16 i, sizeWithoutCRC;
	Uint16 * transmitStringWithoutCRC;

	// Reference to MODBUS Data Map
	char * dataPtr;
	Uint16 sizeOfMap = 0;

	master->dataRequest.slaveAddress = requester.slaveAddress;
	master->dataRequest.functionCode = requester.functionCode;

	// First, get the right data map based on the function code
	if (requester.functionCode == MB_FUNC_READ_COIL ||
			requester.functionCode == MB_FUNC_FORCE_COIL ||
			requester.functionCode == MB_FUNC_FORCE_NCOILS)
	{
		dataPtr = (char *)&(master->coils);
		sizeOfMap = sizeof(master->coils);
	}
	else if (requester.functionCode == MB_FUNC_READ_INPUT)
	{
		dataPtr = (char *)&(master->inputs);
		sizeOfMap = sizeof(master->inputs);
	}
	else if (requester.functionCode == MB_FUNC_READ_HOLDINGREGISTERS ||
			requester.functionCode == MB_FUNC_WRITE_HOLDINGREGISTER ||
			requester.functionCode == MB_FUNC_WRITE_NREGISTERS)
	{
		dataPtr = (char *)&(master->holdingRegisters);
		sizeOfMap = sizeof(master->holdingRegisters);
	}
	else if (requester.functionCode == MB_FUNC_READ_INPUTREGISTERS)
	{
		dataPtr = (char *)&(master->inputRegisters);
		sizeOfMap = sizeof(master->inputRegisters);
	}


	// Second: prepare the dataRequest content array
	if (requester.functionCode == MB_FUNC_READ_COIL || requester.functionCode == MB_FUNC_READ_INPUT ||
			requester.functionCode == MB_FUNC_READ_HOLDINGREGISTERS || requester.functionCode == MB_FUNC_READ_INPUTREGISTERS)
	{
		master->dataRequest.content[MB_READ_ADDRESS_HIGH]   = (requester.addr & 0xFF00) >> 8;
		master->dataRequest.content[MB_READ_ADDRESS_LOW]    = (requester.addr & 0x00FF);
		master->dataRequest.content[MB_READ_TOTALDATA_HIGH] = (requester.totalData & 0xFF00) >> 8;
		master->dataRequest.content[MB_READ_TOTALDATA_LOW]  = (requester.totalData & 0x00FF);
		master->dataRequest.contentIdx = MB_READ_TOTALDATA_LOW;
	}
	else if (requester.functionCode == MB_FUNC_WRITE_HOLDINGREGISTER || requester.functionCode == MB_FUNC_FORCE_COIL) {
		master->dataRequest.content[MB_WRITE_ADDRESS_HIGH]  = (requester.addr & 0xFF00) >> 8;
		master->dataRequest.content[MB_WRITE_ADDRESS_HIGH]  = (requester.addr & 0x00FF);
		master->dataRequest.content[MB_WRITE_VALUE_HIGH]    = (requester.content[0] & 0xFF00) >> 8;
		master->dataRequest.content[MB_WRITE_VALUE_LOW]     = (requester.content[0] & 0x00FF);
		master->dataRequest.contentIdx = MB_WRITE_VALUE_LOW;

		// Get the data at the specified address (one byte only)
		master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + requester.addr + 1) & 0xFF00) >> 8;
		master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + requester.addr + 1) & 0x00FF);
	}
	else if (requester.functionCode == MB_FUNC_WRITE_NREGISTERS ||	requester.functionCode == MB_FUNC_FORCE_NCOILS) {
		master->dataRequest.content[MB_WRITE_N_ADDRESS_HIGH]  = (requester.addr & 0xFF00) >> 8;
		master->dataRequest.content[MB_WRITE_N_ADDRESS_LOW]  = (requester.addr & 0x00FF);

		master->dataRequest.content[MB_WRITE_N_QUANTITY_HIGH] = (requester.totalData & 0xFF00) >> 8;
		master->dataRequest.content[MB_WRITE_N_QUANTITY_LOW]  = (requester.totalData & 0x00FF);
		master->dataRequest.content[MB_WRITE_N_BYTES]         = (requester.totalData) * 2;
		master->dataRequest.contentIdx = MB_WRITE_N_VALUES_START_HIGH;

		// Loop throught the selected data map
		for(i=0; i < requester.totalData; i++) {
			Uint16 padding = i + requester.addr;
			master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + padding + 1) & 0xFF00) >> 8;
			master->dataRequest.content[master->dataRequest.contentIdx++] = (*(dataPtr + padding + 1) & 0x00FF);
		}
	}

	// Prepares data request string
	master->dataRequest.size = MB_SIZE_COMMON_DATA + master->dataRequest.contentIdx;
	sizeWithoutCRC = master->dataRequest.size - 2;
	transmitStringWithoutCRC = master->dataResponse.getTransmitStringWithoutCRC(&master->dataRequest);
	master->dataRequest.crc = generateCrc( transmitStringWithoutCRC, sizeWithoutCRC, true );

	master->requester.isReady = true;
}

ModbusRequester construct_ModbusRequestHandler(){
	ModbusRequester requestHandler;

	requestHandler.slaveAddress = 1;
	requestHandler.functionCode = MB_FUNC_READ_HOLDINGREGISTERS;
	requestHandler.addr = 0;
	requestHandler.totalData = 0;

	requestHandler.isReady = false;

	requestHandler.generate = requester_generate;
	requestHandler.setContent = requester_setContent;

	MB_DATA_HANDLER_DEBUG();

	return requestHandler;
}
