#include "ModbusMaster.h"
#include "DSP2833x_GlobalPrototypes.h"
#include "DSP2833x_Device.h"

ModbusMaster mb;

int main(){
	Uint16 testVar = 0;
	mb = construct_ModbusMaster();

	while(1) {
		Uint16 registerContents[2];

		testVar++;
		if (testVar == 256) testVar = 0;

		// Implementing a demo request using mb.requestHandler
		mb.requestHandler.slaveAddress = 0x01;
		mb.requestHandler.functionCode = MB_FUNC_WRITE_NREGISTERS;
		mb.requestHandler.firstAddr	   = 0x02;
		mb.requestHandler.totalData    = 2;

		registerContents[0] = testVar;
		registerContents[1] = 0xFFFF;
		mb.requestHandler.content      = registerContents;

		mb.requestHandler.generate(&mb);

		mb.loopStates(&mb);
	}
}
