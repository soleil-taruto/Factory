/*
	tSec == 0 ‚Í 1/1/1 Œ—j“ú 00:00:00
*/

#include "all.h"

#define START_TIME (0 * 86400 + 7 * 3600 +  0 * 60) // Œ—j“ú(0“úŒã) 07:00:00
#define END_TIME   (5 * 86400 + 5 * 3600 + 50 * 60) // “y—j“ú(5“úŒã) 05:50:00
#define TIME_CYCLE (7 * 86400 + 0 * 3600 +  0 * 60) // ‚PTŠÔ

#define TRADING_TIME (END_TIME - START_TIME) // æˆøŠÔ
#define INTERVAL_TIME (TIME_CYCLE - TRADING_TIME) // ‚¨‹x‚İŠÔ

#define EPOCH_ZERO_FXTIME 43951122600ui64

uint64 FxTime2TSec(uint fxTime)
{
	uint64 tmp = (uint64)fxTime + EPOCH_ZERO_FXTIME;
	uint64 count;
	uint64 rem;

	count = tmp / TRADING_TIME;
	rem   = tmp % TRADING_TIME;

	return count * TIME_CYCLE + rem + START_TIME;
}
uint TSec2FxTime(uint64 tSec)
{
	uint64 count = tSec / TIME_CYCLE;
	uint64 rem   = tSec % TIME_CYCLE;
	uint64 tmp;

	errorCase(rem < START_TIME || END_TIME <= rem);

	tmp = count * TRADING_TIME + rem - START_TIME;
	tmp -= EPOCH_ZERO_FXTIME;

	return (uint)tmp;
}
