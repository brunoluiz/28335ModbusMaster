#include "PlataformSettings.h"
#include "DSP2833x_GlobalPrototypes.h"
#include "DSP2833x_PieCtrl.h"
#include "DSP2833x_PieVect.h"
#include "DSP2833x_DefaultISR.h"
#include "ModbusMaster.h"

int main(){
	InitSysCtrl();

	DINT;

	InitPieCtrl();

	IER = 0x0000;
	IFR = 0x0000;

	InitPieVectTable();

	mb = construct_ModbusMaster();

	while(1) {
		mb.loopStates(&mb);
	}
}
