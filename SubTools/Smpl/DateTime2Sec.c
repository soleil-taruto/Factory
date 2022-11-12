// smpl
/*
	DateTime2Sec.exe /DT2S リソース日時

		リソース日時 == MMDD, YYYY, YYYYMM, YYYYMMDD, YYYYMMDDhh, YYYYMMDDhhmm, YYYYMMDDhhmmss
		対応する 0001/01/01 00:00:00 からの経過秒数を表示する。

	DateTime2Sec.exe /S2DT 秒数

		秒数 == 0001/01/01 00:00:00 からの経過秒数
		対応する YYYYMMDDhhmmss を表示する。

	Datetime2Sec.exe PRM

		PRM < 10000101000000 の場合

			PRM を 0001/01/01 00:00:00 からの経過秒数と見なして、
			対応する YYYYMMDDhhmmss を表示する。

		PRM >= 10000101000000 の場合

			PRM を YYYYMMDDhhmmss と見なして、
			対応する 0001/01/01 00:00:00 からの経過秒数を表示する。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\TimeData.h"

int main(int argc, char **argv)
{
	if (argIs("/DT2S"))
	{
		cout("%I64u\n", TimeData2TSec(Res2TimeData(nextArg())));
		return;
	}
	if (argIs("/S2DT"))
	{
		cout("%I64u\n", TimeData2Stamp(TSec2TimeData(toValue64(nextArg()))));
		return;
	}

	{
		uint64 prm = toValue64(nextArg());

		if (prm < 10000101000000ui64)
		{
			cout("%I64u\n", TimeData2Stamp(TSec2TimeData(prm)));
		}
		else
		{
			cout("%I64u\n", TimeData2TSec(Stamp2TimeData(prm)));
		}
	}
}
