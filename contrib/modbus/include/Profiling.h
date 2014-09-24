#ifndef PROFILING_H_
#define PROFILING_H_

#include "ModbusSettings.h"

#define PROFILING_TIME_MEASURES_SIZE	10

typedef struct ProfilingTool ProfilingTool;

struct ProfilingTool {
	Uint64 timeMeasures[PROFILING_TIME_MEASURES_SIZE];

	Uint32 endTimeHigh;
	Uint32 endTimeLow;

	Uint32 startTimeHigh;
	Uint32 startTimeLow;

	Uint16 timeMeasuresIndex;

	void (*start)(ProfilingTool * self);
	void (*stop)(ProfilingTool * self);
};

inline void profiling_start();
inline void profiling_stop();
ProfilingTool construct_ProfilingTool();

#endif
