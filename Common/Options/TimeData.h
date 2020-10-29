#pragma once

#include "C:\Factory\Common\all.h"
#include "Date2Day.h"

typedef struct TimeData_st
{
	uint Y; // 1 Å` 2^32-1
	uint M; // 1 Å` 12
	uint D; // 1 Å` 31
	uint W; // 0 Å` 6 == åé Å` ì˙
	uint H; // 0 Å` 23
	uint I; // 0 Å` 59
	uint S; // 0 Å` 59
}
TimeData_t;

TimeData_t GetTimeData(uint y, uint m, uint d, uint h, uint i, uint s);

TimeData_t TSec2TimeData(uint64 tSec);
uint64 TimeData2TSec(TimeData_t td);

TimeData_t Stamp2TimeData(uint64 stamp);
uint64 TimeData2Stamp(TimeData_t td);

uint64 Epoch2TSec(time_t t);
time_t TSec2Epoch(uint64 tSec);

TimeData_t GetNowTimeData(void);
uint64 GetNowTSec(void);
uint64 GetNowStamp(void);
time_t GetNowEpoch(void);

TimeData_t ResStamp2TimeData(uint64 prmStamp);
TimeData_t Res2TimeData(char *prm);
TimeData_t Res2TimeData_x(char *prm);
