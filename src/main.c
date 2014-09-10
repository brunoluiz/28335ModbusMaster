#include "PlataformSettings.h"
#include "ModbusMaster.h"

ModbusMaster mb;

int main(){
	InitSysCtrl();

	mb = construct_ModbusMaster();

	while(1) {
		mb.loopStates(&mb);
	}
}
