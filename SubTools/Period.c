/*
	Period.exe [/EI] ID [/V | /CTT | /C | /R MIN-SEC MAX-SEC | /RH MIN-HOUR MAX-HOUR | /T SEC | /TM MINUTE | /TH HOUR | /TD DAY]

		ID ... �啶������������ʂ���B
			/CTT ���w�肷��ꍇ�́A�g�p���Ȃ��B�K���ȕ�������Z�b�g���邱�ƁB

		/V ... �\���̂�

	----
	��

	�S�N���A

		Period a /ctt

	�\���̂�

		Period a /v
		Period a /t 0   ... �荏�ɒB���Ă��A�������ōĐݒ肳���̂Łu�荏�ɒB�����v��Ԃ͈ێ������B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

static int EqualInterval;
static time_t P_CurrTime;
static char *S_Id;

// ---- time table ----

#define TIME_TABLE_FILE "C:\\Factory\\tmp\\Period.dat"
//#define TIME_TABLE_FILE "C:\\appdata\\Period.dat" // appdata �}�~ @ 2021.3.19

static autoList_t *TTbl;
static uint TTPos;

static void ReadTimeTable(void)
{
	autoList_t *lines;
	uint index;

	if (existFile(TIME_TABLE_FILE))
		lines = readLines(TIME_TABLE_FILE);
	else
		lines = newList();

	for (index = 0; index < getCount(lines); index += 2)
	{
		char *id = getLine(lines, index);

		if (!strcmp(id, S_Id))
			break;
	}
	if (TTbl)
		releaseDim(TTbl, 1);

	TTbl = lines;
	TTPos = index;
}
static void SaveTimeTable(void)
{
	writeLines(TIME_TABLE_FILE, TTbl);
}

// ---- time table tools ----

static time_t GetTime(void)
{
	ReadTimeTable();

	if (TTPos < getCount(TTbl)) // �������B
	{
		char *st = getLine(TTbl, TTPos + 1);
		time_t time;

		time = (time_t)toValue64(st);

		return time;
	}
	return -1; // ���������B
}
static void SetTime(time_t time)
{
	ReadTimeTable();

	if (TTPos < getCount(TTbl)) // ����
	{
		memFree(zSetLine(TTbl, TTPos + 1, xcout("%I64d", time)));
	}
	else // �V�K
	{
		addElement(TTbl, (uint)strx(S_Id));
		addElement(TTbl, (uint)xcout("%I64d", time));
	}
	SaveTimeTable();
}
static void ClearTime(void)
{
	ReadTimeTable();

	if (TTPos < getCount(TTbl)) // �������B
	{
		memFree(getLine(TTbl, TTPos + 0));
		memFree(getLine(TTbl, TTPos + 1));

		setElement(TTbl, TTPos + 0, 0);
		setElement(TTbl, TTPos + 1, 0);

		removeZero(TTbl);

		SaveTimeTable();
	}
}

// ----

static void CheckTime(void)
{
	time_t nextTime = GetTime();
	time_t currTime = P_CurrTime;

	cout("���񎞍��F%f\n", nextTime / 3600.0);
	cout("���ݎ����F%f\n", currTime / 3600.0);

	if (currTime < nextTime)
	{
		cout("���̎����܂ł��� %f ���Ԃ���܂��BendCode=1\n", (nextTime - currTime) / 3600.0);

		termination(1); // ���Ԃ܂�
	}
}
static void UpdateTime(time_t addTime)
{
	time_t nextTime = P_CurrTime + addTime;

	cout("���ԃL�^�I���̎����܂� %f ���Ԃł��BendCode=0\n", addTime / 3600.0);
	cout("���񎞍��F%f\n", nextTime / 3600.0);

	if (EqualInterval)
	{
		nextTime -= nextTime % addTime;

		cout("���񎞍��F%f (�C��)\n", nextTime / 3600.0);
	}

	SetTime(nextTime);

	termination(0); // ���ԃL�^
}
static void UpdateTime_NoUpdate(void)
{
	cout("���ԃL�^�I�\���̂�\n");

	termination(0); // ���ԃL�^
}

// ---- UpdateTime() wrap ----

static void Update_Range(uint minSec, uint maxSec)
{
	errorCase(maxSec < minSec);

	UpdateTime(mt19937_range(minSec, maxSec));
}
static void Update_RangeHour(uint minHour, uint maxHour)
{
	errorCase(maxHour < minHour);

	UpdateTime((time_t)mt19937_range(minHour, maxHour) * 3600);
}
static void Update_Time(uint sec)
{
	UpdateTime(sec);
}
static void Update_TimeMinute(uint minute)
{
	UpdateTime((time_t)minute * 60);
}
static void Update_TimeHour(uint hour)
{
	UpdateTime((time_t)hour * 3600);
}
static void Update_TimeDay(uint day)
{
	UpdateTime((time_t)day * 86400);
}

// ----

int main(int argc, char **argv)
{
	mt19937_initCRnd();
//	mkAppDataDir(); // appdata �}�~ @ 2021.3.19

	if (argIs("/EI"))
	{
		EqualInterval = 1;
	}
	P_CurrTime = time(NULL) + 9 * 3600; // JST
	S_Id = nextArg();

	errorCase(m_isEmpty(S_Id));

	if (argIs("/CTT"))
	{
		cout("Clear Time Table\n");
		removeFileIfExist(TIME_TABLE_FILE);
		return;
	}
	if (argIs("/C"))
	{
		cout("Clear\n");
		ClearTime();
		return;
	}

	CheckTime();

	if (argIs("/V"))
	{
		UpdateTime_NoUpdate();
	}
	if (argIs("/R"))
	{
		uint minSec;
		uint maxSec;

		minSec = toValue(nextArg());
		maxSec = toValue(nextArg());

		Update_Range(minSec, maxSec);
	}
	if (argIs("/RH"))
	{
		uint minHour;
		uint maxHour;

		minHour = toValue(nextArg());
		maxHour = toValue(nextArg());

		Update_RangeHour(minHour, maxHour);
	}
	if (argIs("/T"))
	{
		uint sec = toValue(nextArg());

		Update_Time(sec);
	}
	if (argIs("/TM"))
	{
		uint minute = toValue(nextArg());

		Update_TimeMinute(minute);
	}
	if (argIs("/TH"))
	{
		uint hour = toValue(nextArg());

		Update_TimeHour(hour);
	}
	if (argIs("/TD"))
	{
		uint day = toValue(nextArg());

		Update_TimeDay(day);
	}
	Update_Time(60);
}
