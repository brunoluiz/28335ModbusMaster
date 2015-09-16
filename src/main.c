#include "ModbusMaster.h"
#include "DSP2833x_GlobalPrototypes.h"
#include "DSP2833x_Device.h"

ModbusMaster mb;

int main(){
	Uint16 testVar = 0;
	InitSysCtrl();
	mb = construct_ModbusMaster();

	mb.requester.slaveAddress = 0x01;
	mb.requester.functionCode = MB_FUNC_WRITE_NREGISTERS;
	mb.requester.addr	   = 0x00;
	mb.requester.totalData    = 2;

	while(1) {
		mb.requester.generate(&mb);
		mb.loopStates(&mb);
	}
}
