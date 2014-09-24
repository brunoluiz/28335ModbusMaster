/*
 * Profiling.c
 *
 *  Created on: 24/09/2014
 *      Author: bds
 */
#include "Profiling.h"
#include "DSP2833x_CpuTimers.h"

void profiling_start(ProfilingTool * self) {
	// DEBUG: Start Debug Timer
	self->startTimeHigh = CpuTimer2Regs.TIM.half.LSW;
	self->startTimeLow = CpuTimer2Regs.TIM.half.MSW;
	CpuTimer2Regs.TCR.bit.TSS = 0;
}

void profiling_stop(ProfilingTool * self) {
	// DEBUG: Stop Debug Timer
	CpuTimer2Regs.TCR.bit.TSS = 1;

	self->endTimeHigh = CpuTimer2Regs.TIM.half.LSW;
	self->endTimeLow = CpuTimer2Regs.TIM.half.MSW;

	if (self->timeMeasuresIndex >= PROFILING_TIME_MEASURES_SIZE)
		self->timeMeasuresIndex = 0;

	// Save profile time measure
	self->timeMeasures[self->timeMeasuresIndex] =
			( (self->startTimeHigh - self->endTimeHigh) + self->startTimeHigh*(self->startTimeLow - self->endTimeLow) ) / 150000;
	// Divided by 150kHz instead of 150Mhz (real clock) because then timeMeasures will be at milliseconds

	// Reset timer
	CpuTimer2Regs.TCR.bit.TRB = 1;

	self->timeMeasuresIndex++;
}

ProfilingTool construct_ProfilingTool(){
	ProfilingTool profiling;

	profiling.endTimeHigh = 0;
	profiling.endTimeLow = 0;

	profiling.startTimeHigh = 0;
	profiling.startTimeLow = 0;

	profiling.timeMeasuresIndex = 0;

	profiling.start = profiling_start;
	profiling.stop = profiling_stop;

	ConfigCpuTimer(&CpuTimer2, 150, 1000000);
	CpuTimer2Regs.TCR.bit.TRB = 1;

	return profiling;
}
