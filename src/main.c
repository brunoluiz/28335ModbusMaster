#include "ModbusMaster.h"
#include "DSP2833x_GlobalPrototypes.h"
#include "DSP2833x_Device.h"

ModbusMaster mb;

int main(){
	mb = construct_ModbusMaster();

	while(1) {
		mb.loopStates(&mb);
	}
}
