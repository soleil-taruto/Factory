/*
	PwOnTime.exe [/POS PW-OFF-SEC] ...

		PW-OFF-SEC ... 電源オフと見なすイベントの間隔（秒数）

	PwOnTime.exe ...

		今月の稼働状況を表示

	PwOnTime.exe ... /L

		先月の稼働状況を表示

	PwOnTime.exe ... YYYYMM

		指定された月の稼働状況を表示

		ex.
			PwOnTime 199907

	PwOnTime.exe ... YYYYMM yyyymm

		指定された範囲（月単位）の稼働状況を表示

		ex.
			PwOnTime 201601 201612
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Date2Day.h"
#include "C:\Factory\Meteor\Toolkit.h"

static uint64 PwOffSec = 3 * 3600;

static uint YMin;
static uint MMin;
static uint YMax;
static uint MMax;

static char *ToPrStamp(char *stamp)
{
	return makeJStamp(getStampDataTime(compactStampToTime(stamp)), 0);
}
static char *ToPrStampSpan(char *stamp1, char *stamp2)
{
	time_t t1 = compactStampToTime(stamp1);
	time_t t2 = compactStampToTime(stamp2);
	uint t;

	t = (uint)(t2 - t1);

	return xcout("%u 時間 %02u 分 %02u 秒", t / 3600, (t / 60) % 60, t % 60);
}
static void ShowPwOnTime(void)
{
	char *file = makeTempPath(NULL);
	autoList_t *lines;
	char *line;
	uint index;
	autoList_t *pwOffSpans = newList();
	autoList_t *pwOnSpans = newList();

	errorCase(!existFile(FILE_TOOLKIT_EXE)); // 外部コマンド存在確認

	LOGPOS();
	coExecute_x(xcout(FILE_TOOLKIT_EXE " /EVENT-LOG %04u%02u01000000 %04u%02u31235959 0 \"%s\"", YMin, MMin, YMax, MMax, file));
	LOGPOS();

	lines = readLines(file);

	foreach (lines, line, index)
	{
		errorCase(!lineExp("<14,09>,<>", line));

		line[14] = '\0';
	}
	addElement(lines, (uint)strx("10000101000000"));
	addElement(lines, (uint)strx("99991231235959"));

	rapidSortLines(lines);

	for (index = 0; index + 1 < getCount(lines); index++)
	{
		char *sDt1 = getLine(lines, index + 0);
		char *sDt2 = getLine(lines, index + 1);
		uint64 dt1;
		uint64 dt2;
		uint64 sec1;
		uint64 sec2;
		uint64 secDiff;

		dt1 = toValue64(sDt1);
		dt2 = toValue64(sDt2);
		sec1 = IDateTime2Sec(dt1);
		sec2 = IDateTime2Sec(dt2);
		secDiff = sec2 - sec1;

		if (PwOffSec <= secDiff)
		{
			addElement(pwOffSpans, (uint)sDt1);
			addElement(pwOffSpans, (uint)sDt2);
		}
	}
	cout("----\n");

	for (index = 0; index < getCount(pwOffSpans); index += 2)
	{
		cout("%s から %s まで電源オフでした。\n",
			getLine(pwOffSpans, index + 0),
			getLine(pwOffSpans, index + 1)
			);
	}
	cout("----\n");

	pwOnSpans = copyAutoList(pwOffSpans);
	unaddElement(pwOnSpans);
	desertElement(pwOnSpans, 0);

	for (index = 0; index < getCount(pwOnSpans); index += 2)
	{
		cout("%s から %s まで電源オンでした。\n",
			getLine(pwOnSpans, index + 0),
			getLine(pwOnSpans, index + 1)
			);
	}
	cout("----\n");

	for (index = 0; index < getCount(pwOnSpans); index += 2)
	{
		cout(
			"\n"
			"%s から\n"
			"%s まで電源オンでした。===> %s\n",
			ToPrStamp(getLine(pwOnSpans, index + 0)), // g
			ToPrStamp(getLine(pwOnSpans, index + 1)), // g
			ToPrStampSpan(getLine(pwOnSpans, index + 0), getLine(pwOnSpans, index + 1)) // g
			);
	}
	releaseDim(lines, 1);
	removeFile(file);
	memFree(file);
	releaseAutoList(pwOffSpans);
	releaseAutoList(pwOnSpans);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/POS"))
	{
		PwOffSec = toValue64(nextArg());
		goto readArgs;
	}

	if (argIs("/L")) // 先月
	{
		char *stamp = makeCompactStamp(NULL);
		uint y;
		uint m;

		stamp[6] = '\0';

		m = toValue(stamp);
		y = m / 100;
		m %= 100;

		if (m <= 1)
		{
			y--;
			m = 12;
		}
		else
		{
			m--;
		}

		YMin = y;
		MMin = m;
		YMax = y;
		MMax = m;

		ShowPwOnTime();
		return;
	}

	if (hasArgs(2))
	{
		uint ym1;
		uint ym2;

		ym1 = toValue(nextArg());
		ym2 = toValue(nextArg());

		errorCase(!m_isRange(ym1, 100001, 999912));
		errorCase(!m_isRange(ym2, 100001, 999912));
		errorCase(ym2 < ym1);

		YMin = ym1 / 100;
		MMin = ym1 % 100;
		YMax = ym2 / 100;
		MMax = ym2 % 100;

		ShowPwOnTime();
		return;
	}
	if (hasArgs(1))
	{
		uint ym = toValue(nextArg());

		errorCase(!m_isRange(ym, 100001, 999912));

		YMin = ym / 100;
		MMin = ym % 100;
		YMax = ym / 100;
		MMax = ym % 100;

		ShowPwOnTime();
		return;
	}

	{
		char *stamp = makeCompactStamp(NULL);
		uint y;
		uint m;

		stamp[6] = '\0';

		m = toValue(stamp);
		y = m / 100;
		m %= 100;

		YMin = y;
		MMin = m;
		YMax = y;
		MMax = m;

		ShowPwOnTime();
		return;
	}
}
