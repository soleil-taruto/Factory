/*
	TSec (time second) == ローカル時間 0001/01/01 00:00:00 からの経過秒数

	Epoch, Epoch Time == ローカル時間 1970/01/01 00:00:00 からの経過秒数

	time(NULL) は UTC の 1970/01/01 00:00:00 からの経過秒数であることに注意！
*/

#include "TimeData.h"

#define EPOCH_ZERO_TSEC 62135596800i64

TimeData_t GetTimeData(uint y, uint m, uint d, uint h, uint i, uint s)
{
	TimeData_t td;

	td.Y = y;
	td.M = m;
	td.D = d;
	td.W = Date2Day(y, m, d) % 7;
	td.H = h;
	td.I = i;
	td.S = s;

	return td;
}

TimeData_t TSec2TimeData(uint64 tSec)
{
	TimeData_t td;

	td.S = tSec % 60;
	tSec /= 60;
	td.I = tSec % 60;
	tSec /= 60;
	td.H = tSec % 24;
	tSec /= 24;

	Day2Date(tSec, &td.Y, &td.M, &td.D);

	td.W = (uint)(tSec % 7);

	return td;
}
uint64 TimeData2TSec(TimeData_t td)
{
	uint64 tSec = Date2Day(td.Y, td.M, td.D);

	tSec *= 24;
	tSec += td.H;
	tSec *= 60;
	tSec += td.I;
	tSec *= 60;
	tSec += td.S;

	return tSec;
}

TimeData_t Stamp2TimeData(uint64 stamp)
{
	TimeData_t td;

	td.S = stamp % 100;
	stamp /= 100;
	td.I = stamp % 100;
	stamp /= 100;
	td.H = stamp % 100;
	stamp /= 100;
	td.D = stamp % 100;
	stamp /= 100;
	td.M = stamp % 100;
	td.Y = stamp / 100;

	td.W = Date2Day(td.Y, td.M, td.D) % 7;

	return td;
}
uint64 TimeData2Stamp(TimeData_t td)
{
	uint64 stamp = td.Y;

	stamp *= 100;
	stamp += td.M;
	stamp *= 100;
	stamp += td.D;
	stamp *= 100;
	stamp += td.H;
	stamp *= 100;
	stamp += td.I;
	stamp *= 100;
	stamp += td.S;

	return stamp;
}

uint64 Epoch2TSec(time_t t)
{
	return (uint64)t + EPOCH_ZERO_TSEC;
}
time_t TSec2Epoch(uint64 tSec)
{
	return (time_t)(tSec - EPOCH_ZERO_TSEC);
}

TimeData_t GetNowTimeData(void)
{
	stampData_t *sd = getStampDataTime(0);
	TimeData_t td;

	td.Y = sd->year;
	td.M = sd->month;
	td.D = sd->day;
	td.W = (sd->weekday + 6) % 7;
	td.H = sd->hour;
	td.I = sd->minute;
	td.S = sd->second;

	return td;
}
uint64 GetNowTSec(void)
{
	return TimeData2TSec(GetNowTimeData());
}
uint64 GetNowStamp(void)
{
	return TimeData2Stamp(GetNowTimeData());
}
time_t GetNowEpoch(void) // == time(NULL) + ローカル時間
{
	return TSec2Epoch(GetNowTSec());
}

TimeData_t ResStamp2TimeData(uint64 prmStamp)
{
	uint64 stamp = prmStamp;
	TimeData_t td;

	if (stamp <= 1231) // ? MMDD
	{
		stamp += GetNowTimeData().Y * 10000;
		stamp *= 1000000;
	}
	else if (stamp < 10000) // ? YYYY
	{
		stamp *= 10000;
		stamp += 101;
		stamp *= 1000000;
	}
	else if (stamp < 1000000) // ? YYYYMM
	{
		stamp *= 100;
		stamp += 1;
		stamp *= 1000000;
	}
	else if (stamp < 100000000) // ? YYYYMMDD
	{
		stamp *= 1000000;
	}
	else if (stamp < 10000000000) // ? YYYYMMDDHH
	{
		stamp *= 10000;
	}
	else if (stamp < 1000000000000) // ? YYYYMMDDHHMM
	{
		stamp *= 100;
	}

	td = Stamp2TimeData(stamp);
	td = TSec2TimeData(TimeData2TSec(td)); // fltr

	cout("%I64u -> %04u/%02u/%02u %02u:%02u:%02u\n", prmStamp, td.Y, td.M, td.D, td.H, td.I, td.S); // debug-out

	return td;
}
TimeData_t Res2TimeData(char *prm)
{
	return ResStamp2TimeData(toValue64(prm));
}
TimeData_t Res2TimeData_x(char *prm)
{
	TimeData_t out = Res2TimeData(prm);
	memFree(prm);
	return out;
}
