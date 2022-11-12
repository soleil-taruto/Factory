/*
	getStampData
	getStampDataTime

		getStampData, getStampDataTime は LastStampData へのポインタを返す。

		複数箇所で同じもの(LastStampData)を読み書きして酷い目にあわない為に、
		stampData_t *i = getStampData(NULL);
		ではなく
		stampData_t i = *getStampData(NULL);
		で受け取るようにすること。極力。

		このファイル内の関数の引数に渡す時は直でokとする。
		makeCompactStamp(getStampData(NULL));
*/

#include "all.h"

static char *MonthList[] = {
	"Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec",
};
static char *WeekDayList[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};
static char *JWeekDayList[] = {
	"日", "月", "火", "水", "木", "金", "土",
};

char *getEMonth(uint month)
{
	if (1 <= month && month <= 12) return MonthList[month - 1];
	return "???";
}
char *getEWeekDay(uint weekday)
{
	if (weekday <= 6) return WeekDayList[weekday];
	return "???";
}
char *getJWeekDay(uint weekday)
{
	if (weekday <= 6) return JWeekDayList[weekday];
	return "？";
}

/*
	ctime() は 0 ～ 32535244799 (0x793406fff) -> Thu Jan 01 09:00:00 1970 ～ Thu Jan 01 16:59:59 3001 を受け付ける。日本なので GMT + 9時間
	それ以外は NULL を返す。
*/
char *makeStamp(time_t t) // t == 0: 現時刻
{
	char *stamp;

	if (!t)
		t = time(NULL);

	stamp = ctime(&t); // "Wed Jan 02 02:03:55 1980\n"

	if (!stamp)
		stamp = "Thu Jan 01 00:00:00 1970\n"; // 不正な t

	stamp = strx(stamp);
	stamp[24] = '\0';

	return stamp; // "Wed Jan 02 02:03:55 1980"
}

static stampData_t LastStampData;

static void UpdateStampData(char *stamp)
{
	stampData_t *i = &LastStampData;
	autoList_t tmplist;

	errorCase(!stamp);
	errorCase(strlen(stamp) != 24);

	stamp = strx(stamp);

	stamp[3] = '\0';
	stamp[7] = '\0';
	stamp[10] = '\0';
	stamp[13] = '\0';
	stamp[16] = '\0';
	stamp[19] = '\0';

	i->year = toValue(stamp + 20);
	i->month = findLine(gndAutoListVar((uint *)MonthList, lengthof(MonthList), tmplist), stamp + 4) + 1;
	i->day = toValue(stamp + 8);
	i->weekday = findLine(gndAutoListVar((uint *)WeekDayList, lengthof(WeekDayList), tmplist), stamp);
	i->hour = toValue(stamp + 11);
	i->minute = toValue(stamp + 14);
	i->second = toValue(stamp + 17);

	memFree(stamp);
}
static void UpdateStampDataTime(time_t t)
{
	char *stamp = makeStamp(t);

	UpdateStampData(stamp);
	memFree(stamp);
}
stampData_t *getStampData(char *stamp)
{
	UpdateStampData(stamp);
	return &LastStampData;
}
stampData_t *getStampDataTime(time_t t)
{
	UpdateStampDataTime(t);
	return &LastStampData;
}

int isAllowStampData(stampData_t *i)
{
	// メンバー毎に ctime() が返し得る値かチェックする。
	return
		1970 <= i->year    && i->year    <= 3001 &&
		   1 <= i->month   && i->month   <=   12 &&
		   1 <= i->day     && i->day     <=   31 &&
		   0 <= i->weekday && i->weekday <=    6 &&
		   0 <= i->hour    && i->hour    <=   23 &&
		   0 <= i->minute  && i->minute  <=   59 &&
		   0 <= i->second  && i->second  <=   59;
}

char *makeJStamp(stampData_t *i, int nonWeekDay)
{
	char *stamp;

	if (!i)
		i = getStampDataTime(0);

	if (!isAllowStampData(i))
		return strx("1970/01/01 (木) 00:00:00"); // 不正な i

	stamp = xcout(
		"%04u/%02u/%02u (%s) %02u:%02u:%02u",
		i->year,
		i->month,
		i->day,
		JWeekDayList[i->weekday],
		i->hour,
		i->minute,
		i->second
		);

	// "1980/01/02 (水) 02:03:55"

	if (nonWeekDay)
	{
		eraseLine(stamp + 11, 5); // "1980/01/02 02:03:55"
	}
	return stamp;
}
char *makeCompactStamp(stampData_t *i)
{
	char *stamp;

	if (!i)
		i = getStampDataTime(0);

	if (!isAllowStampData(i))
		return strx("19700101000000"); // 不正な i

	stamp = xcout(
		"%04u%02u%02u%02u%02u%02u",
		i->year,
		i->month,
		i->day,
		i->hour,
		i->minute,
		i->second
		);

	return stamp; // "19800102020355"
}
time_t stampDataToTime(stampData_t *i)
{
	char *stamp = makeCompactStamp(i);
	time_t ret;

	ret = compactStampToTime(stamp);
	memFree(stamp);
	return ret;
}
time_t compactStampToTime(char *stamp)
{
	time_t nt =              -1; // Min - 1
	time_t ft = 0x793406fff + 1; // Max + 1

	while (nt + 2 < ft)
	{
		time_t mt = (nt + ft) / 2;
		char *ms;
		sint comp;

		ms = makeCompactStamp(getStampDataTime(mt));
		comp = strcmp(stamp, ms);
		memFree(ms);

		if (!comp) // ? stamp == ms
		{
			return mt;
		}
		if (comp < 0) // ? stamp < ms
		{
			ft = mt;
		}
		else // ? stamp > ms
		{
			nt = mt;
		}
	}
	return nt + 1; // ft - 1
}

// _x
time_t compactStampToTime_x(char *stamp)
{
	time_t out = compactStampToTime(stamp);
	memFree(stamp);
	return out;
}

// c_
char *c_makeStamp(time_t t)
{
	static char *stock;
	memFree(stock);
	return stock = makeStamp(t);
}
char *c_makeJStamp(stampData_t *i, int nonWeekDay)
{
	static char *stock;
	memFree(stock);
	return stock = makeJStamp(i, nonWeekDay);
}
char *c_makeCompactStamp(stampData_t *i)
{
	static char *stock;
	memFree(stock);
	return stock = makeCompactStamp(i);
}
