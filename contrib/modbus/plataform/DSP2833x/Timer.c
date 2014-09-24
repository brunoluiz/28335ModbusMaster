#include "ModbusMaster.h"
#include "PlataformSettings.h"
#include "DSP2833x_CpuTimers.h"
#include "Timer.h"
#include "Log.h"

interrupt void timer_expiredTimer(void){
	CpuTimer0.InterruptCount++;
	mb.timeout = true;
	mb.timeoutCounter++;
	CpuTimer0Regs.TCR.bit.TSS = 1; 	// Stop timer
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

void timer_resetTimer(){
	CpuTimer0Regs.TCR.bit.TRB = 1;
	TIMER_DEBUG();
}

void timer_setTimerReloadPeriod(Timer *self, Uint32 time){
	TIMER_DEBUG();

	self->stop();
	self->reloadTime = time;

	ConfigCpuTimer(&CpuTimer0, CPU_FREQ, time);
}


void timer_init(Timer *self, Uint32 time){
	EALLOW;  // This is needed to write to EALLOW protected registers
	PieVectTable.TINT0 = &timer_expiredTimer;
	EDIS;
	// Initialize the Device Peripheral. This function can be found in DSP2833x_CpuTimers.c
	InitCpuTimers();

	// Config the timer reload period
	self->setTimerReloadPeriod(self, time);

	// If needed, you can set interruptions and other things here
	CpuTimer0Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
	IER |= M_INT1;
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
	EINT;   // Enable Global interrupt INTM

	TIMER_DEBUG();
}

void timer_stop(){
	CpuTimer0Regs.TCR.bit.TSS = 1;
	TIMER_DEBUG();
}

void timer_start(){
	CpuTimer0Regs.TCR.bit.TSS = 0;
	TIMER_DEBUG();
}

Timer construct_Timer(){
	Timer timer;

	timer.timerEnabled = false;
	timer.reloadTime = 0;

	timer.resetTimer = timer_resetTimer;
	timer.setTimerReloadPeriod = timer_setTimerReloadPeriod;
	timer.init = timer_init;
	timer.stop = timer_stop;
	timer.start = timer_start;

	TIMER_DEBUG();

	return timer;
}
