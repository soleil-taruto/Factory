/*
	time_t をセットするときは、ApplyStampData(getStampDataTime(t)); とする。
	LastStampData が更新されることに注意せよ。
*/
#include "ApplyStampData.h"

void ApplyStampData(stampData_t *i)
{
	/*
		23:59:59.999とかシビアなタイミングだったりすると、
		DATE実行 -> 日付が変わる -> TIME実行で、丸一日ずれたりするかも。
		なのでTIMEを先に。
	*/
	coExecute_x(xcout("TIME %u.%u.%u", i->hour, i->minute, i->second));
	coExecute_x(xcout("DATE %u.%u.%u", i->year, i->month, i->day));
}
void SlewApplyTimeData(time_t trueTime)
{
	time_t systemTime = time(NULL);
	time_t t;

	cout("%I64d -> %I64d\n", systemTime, trueTime); // systemTime から trueTime に近づける。

	if (trueTime == systemTime) // 時刻が合っている。-> 何もしない。
	{
		cout("+0.0\n");
		goto endFunc;
	}

	do
	{
//		sleep(100);
		sleep(10);
	}
	while (systemTime == time(NULL));

	sleep(500); // systemTime + 1.5 まで待つ。

	if (trueTime < systemTime) // システム時刻が進んでいる。-> 0.5 遅らせる。(systemTime + 1.5 -> systemTime + 1.0) == -0.5
	{
		cout("-0.5\n");
		ApplyStampData(getStampDataTime(systemTime + 1));
	}
	else if (systemTime < trueTime) // システム時刻が遅れている。-> 0.5 進める。(systemTime + 1.5 -> systemTime + 2.0) == +0.5
	{
		cout("+0.5\n");
		ApplyStampData(getStampDataTime(systemTime + 2));
	}
endFunc:;
}
