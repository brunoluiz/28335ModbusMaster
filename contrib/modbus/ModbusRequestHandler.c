#include "ModbusDefinitions.h"
#include "ModbusRequestHandler.h"
#include "ModbusMaster.h"
#include "Log.h"
#include "Crc.h"

void requestHandler_setContent(ModbusMaster *master, Uint16 * content, Uint16 length) {
	;
}
void requestHandler_generate(ModbusMaster *master) {
	ModbusRequestHandler requestHandler = master->requestHandler;
	Uint16 i, sizeWithoutCRC;
	Uint16 * transmitStringWithoutCRC;

	master->dataRequest.slaveAddress = requestHandler.slaveAddress;
	master->dataRequest.functionCode = requestHandler.functionCode;

	if (requestHandler.functionCode == MB_FUNC_READ_COIL || requestHandler.functionCode == MB_FUNC_READ_INPUT ||
			requestHandler.functionCode == MB_FUNC_READ_HOLDINGREGISTERS || requestHandler.functionCode == MB_FUNC_READ_INPUTREGISTERS)
	{
		master->dataRequest.content[MB_READ_ADDRESS_HIGH]   = (requestHandler.firstAddr & 0xFF00) >> 8;
		master->dataRequest.content[MB_READ_ADDRESS_LOW]    = (requestHandler.firstAddr & 0x00FF);
		master->dataRequest.content[MB_READ_TOTALDATA_HIGH] = (requestHandler.totalData & 0xFF00) >> 8;
		master->dataRequest.content[MB_READ_TOTALDATA_LOW]  = (requestHandler.totalData & 0x00FF);
		master->dataRequest.contentIdx = MB_READ_TOTALDATA_LOW;
	}
	else if (requestHandler.functionCode == MB_FUNC_WRITE_HOLDINGREGISTER || requestHandler.functionCode == MB_FUNC_FORCE_COIL) {
		master->dataRequest.content[MB_WRITE_ADDRESS_HIGH]  = (requestHandler.firstAddr & 0xFF00) >> 8;
		master->dataRequest.content[MB_WRITE_ADDRESS_HIGH]  = (requestHandler.firstAddr & 0x00FF);
		master->dataRequest.content[MB_WRITE_VALUE_HIGH]    = (requestHandler.content[0] & 0xFF00) >> 8;
		master->dataRequest.content[MB_WRITE_VALUE_LOW]     = (requestHandler.content[0] & 0x00FF);
		master->dataRequest.contentIdx = MB_WRITE_VALUE_LOW;
	}
	else if (requestHandler.functionCode == MB_FUNC_WRITE_NREGISTERS ||	requestHandler.functionCode == MB_FUNC_FORCE_NCOILS) {
		master->dataRequest.content[MB_WRITE_N_ADDRESS_HIGH]  = (requestHandler.firstAddr & 0xFF00) >> 8;
		master->dataRequest.content[MB_WRITE_N_ADDRESS_LOW]  = (requestHandler.firstAddr & 0x00FF);

		master->dataRequest.content[MB_WRITE_N_QUANTITY_HIGH] = (requestHandler.totalData & 0xFF00) >> 8;
		master->dataRequest.content[MB_WRITE_N_QUANTITY_LOW]  = (requestHandler.totalData & 0x00FF);
		master->dataRequest.content[MB_WRITE_N_BYTES]         = (requestHandler.totalData) * 2;
		master->dataRequest.contentIdx = MB_WRITE_N_VALUES_START_HIGH;

		for(i=0; i < requestHandler.totalData; i++) {
			master->dataRequest.content[master->dataRequest.contentIdx++] = (requestHandler.content[i] & 0xFF00) >> 8;
			master->dataRequest.content[master->dataRequest.contentIdx++] = (requestHandler.content[i] & 0x00FF);
		}
	}

	// Prepares data request string
	master->dataRequest.size = MB_SIZE_COMMON_DATA + master->dataRequest.contentIdx;
	sizeWithoutCRC = master->dataRequest.size - 2;
	transmitStringWithoutCRC = master->dataResponse.getTransmitStringWithoutCRC(&master->dataRequest);
	master->dataRequest.crc = generateCrc( transmitStringWithoutCRC, sizeWithoutCRC, true );

	master->requestHandler.isReady = true;
}

ModbusRequestHandler construct_ModbusRequestHandler(){
	ModbusRequestHandler requestHandler;

	requestHandler.slaveAddress = 1;
	requestHandler.functionCode = MB_FUNC_READ_HOLDINGREGISTERS;
	requestHandler.firstAddr = 0;
	requestHandler.totalData = 0;

	requestHandler.isReady = false;

	requestHandler.generate = requestHandler_generate;
	requestHandler.setContent = requestHandler_setContent;

	MB_DATA_HANDLER_DEBUG();

	return requestHandler;
}
