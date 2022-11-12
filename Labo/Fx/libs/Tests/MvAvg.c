#include "C:\Factory\Common\all.h"
#include "..\all.h"

#define PAIR "USDJPY"
#define BGN_STAMP 20160728000000
#define END_STAMP 20160804050000

int main(int argc, char **argv)
{
	uint bgnFxTime = TSec2FxTime(TimeData2TSec(Stamp2TimeData(BGN_STAMP)));
	uint endFxTime = TSec2FxTime(TimeData2TSec(Stamp2TimeData(END_STAMP)));
	uint fxTime;

	for (fxTime = bgnFxTime; fxTime <= endFxTime; fxTime += 2)
	{
		cout("%I64u,%f,%f\n"
			,TimeData2Stamp(TSec2TimeData(FxTime2TSec(fxTime)))
			,MvAvg_GetMid(fxTime, 3600, PAIR)
			,GetMid(GetPrice(fxTime, PAIR))
			);
	}
}
