#include "C:\Factory\Common\all.h"
#include "..\all.h"

#define PAIR "USDJPY"
#define BGN_STAMP 20160728000000
#define END_STAMP 20160803000000

int main(int argc, char **argv)
{
	uint64 bgnTSec = TimeData2TSec(Stamp2TimeData(BGN_STAMP));
	uint64 endTSec = TimeData2TSec(Stamp2TimeData(END_STAMP));
	uint64 tSec;

	for (tSec = bgnTSec; tSec <= endTSec; tSec += 2)
	{
		cout("%I64u,%f\n", TimeData2Stamp(TSec2TimeData(tSec)), GetMid(GetPrice_TSec(tSec, PAIR)));
	}
}
