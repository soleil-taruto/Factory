#include "C:\Factory\Common\all.h"
#include "..\all.h"

#define PAIR "USDJPY"

static void Test01(void)
{
	cout("%I64u\n", TimeData2Stamp(TSec2TimeData(FxTime2TSec(0x00000000))));
	cout("%I64u\n", TimeData2Stamp(TSec2TimeData(FxTime2TSec(0xffffffff))));

	cout("%u\n", TSec2FxTime(TimeData2TSec(Stamp2TimeData(19700101000000))));

	cout("%I64u\n", TimeData2Stamp(TSec2TimeData(FxTime2TSec(0))));
}
static void Test02(void)
{
	uint bgnFxTime = TSec2FxTime(TimeData2TSec(Stamp2TimeData(20160728000000)));
	uint endFxTime = TSec2FxTime(TimeData2TSec(Stamp2TimeData(20160803000000)));
	uint fxTime;

	for (fxTime = bgnFxTime; fxTime <= endFxTime; fxTime += 2)
	{
		uint t;

		cout("%I64u,%I64u,%u,%u\n"
			,TimeData2Stamp(TSec2TimeData(FxTime2TSec(fxTime)))
			,FxTime2TSec(fxTime)
			,fxTime
			,t = TSec2FxTime(FxTime2TSec(fxTime))
			);

		errorCase(t != fxTime);
	}
}

int main(int argc, char **argv)
{
//	Test01();
	Test02();
}
