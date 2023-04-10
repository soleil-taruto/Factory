/*
	rev.exe [/D 年 月 日 時 分 秒 | /E POSIX-Time | /T | /U リビジョン | [/P] [POSIX-Time | 年 月 日 時 分 秒]]

		/D ... 指定日時から POSIX-時間 を表示する。
		/E ... 指定の POSIX-時間 から年月日時分秒を表示する。但し 0 のときは現時刻, 32535244800 以上のときは 1970/1/1 0:0:0 を表示する。
		/T ... 現在の POSIX-時間 を表示する。
		/U ... 指定リビジョンから日時を表示する。
		/P ... 表示のみ。(エディタを開かない)

	----
	書式

	西暦.(100～466).(10000～96399)

		rev_a: 西暦
		rev_b: rev_a の年の 1/1 から何日目か
		rev_c: rev_a, rev_b の日の 0:0:0 から何秒経過したか

		rev_b が 100 未満にならないように rev_a を調整する。
		rev_c が 10000 未満にならないように rev_a, rev_b を調整する。

		rev_b の起算日は 1, rec_c の起算時は 0 であることに注意
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Date2Day.h"

static void MakeRev(time_t revTime, int printOnly) // revTime: 0 == 現時刻
{
	stampData_t *sd = getStampDataTime(revTime);
	uint y;
	uint m;
	uint d;
	uint h;
	uint n;
	uint s;
	uint rev_a;
	uint rev_b;
	uint rev_c;
	char *rev;

	y = sd->year;
	m = sd->month;
	d = sd->day;
	h = sd->hour;
	n = sd->minute;
	s = sd->second;

	rev_a = y;
	rev_b = 1 + Date2Day(y, m, d) - Date2Day(y, 1, 1);
	rev_c = h * 3600 + n * 60 + s;

	if (rev_c < 10000)
	{
		rev_b--;
		rev_c += 86400;
	}
	if (rev_b < 100)
	{
		rev_a--;
		rev_b += Date2Day(rev_a + 1, 1, 1) - Date2Day(rev_a, 1, 1);
	}
	rev = xcout("%u.%u.%u", rev_a, rev_b, rev_c);

	cout("%s\n", rev);

	if (!printOnly)
		viewLineNoRet_NB(rev, 1);

	memFree(rev);
}
static void UnmakeRev(char *rev)
{
	autoList_t *tkns = tokenize(rev, '.');
	uint64 sec;
	uint y;
	uint m;
	uint d;
	uint s;

	errorCase(getCount(tkns) != 3);

	sec = Date2Day(toValue(getLine(tkns, 0)), 1, 1) + toValue(getLine(tkns, 1)) - 1;
	sec *= 86400;
	sec += toValue(getLine(tkns, 2));
	Day2Date(sec / 86400, &y, &m, &d);
	s = sec % 86400;

	cout("%04u/%02u/%02u %02u:%02u:%02u\n"
		,y
		,m
		,d
		,s / 3600
		,(s / 60) % 60
		,s % 60
		);

	releaseDim(tkns, 1);
}
int main(int argc, char **argv)
{
	time_t revTime = 0;
	int printOnly = 0;

	if (argIs("/D"))
	{
		stampData_t sd;

		sd.year = toValue(nextArg());
		sd.month = toValue(nextArg());
		sd.day = toValue(nextArg());
		sd.weekday = 0;
		sd.hour = toValue(nextArg());
		sd.minute = toValue(nextArg());
		sd.second = toValue(nextArg());

		errorCase(!isAllowStampData(&sd));
		cout("%I64d\n", stampDataToTime(&sd));
		return;
	}
	if (argIs("/E"))
	{
		time_t t = (time_t)toValue64(nextArg());
		stampData_t *sd;

		sd = getStampDataTime(t);

		cout("%04u/%02u/%02u (%s) %02u:%02u:%02u\n"
			,sd->year
			,sd->month
			,sd->day
			,getJWeekDay(sd->weekday)
			,sd->hour
			,sd->minute
			,sd->second
			);
		return;
	}
	if (argIs("/T"))
	{
		cout("%I64d\n", time(NULL));
		return;
	}
	if (argIs("/U"))
	{
		UnmakeRev(nextArg());
		return;
	}
	if (argIs("/P"))
	{
		printOnly = 1;
	}
	if (hasArgs(6))
	{
		stampData_t sd;

		sd.year = toValue(nextArg());
		sd.month = toValue(nextArg());
		sd.day = toValue(nextArg());
		sd.weekday = 0;
		sd.hour = toValue(nextArg());
		sd.minute = toValue(nextArg());
		sd.second = toValue(nextArg());

		errorCase(!isAllowStampData(&sd));
		revTime = stampDataToTime(&sd);
	}
	else if (hasArgs(1))
	{
		revTime = toValue64(nextArg());
		errorCase(revTime < 0);
	}
	errorCase_m(hasArgs(1), "不明なコマンド引数");

	MakeRev(revTime, printOnly);
}
