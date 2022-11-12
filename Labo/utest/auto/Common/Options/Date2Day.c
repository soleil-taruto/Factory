#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Date2Day.h"

static uint IsUruu(uint year)
{
	return ( year % 4 ) == 0 && ( year % 100 ) != 0 || ( year % 400 ) == 0; // from Wikipedia
}
static uint GetDayNum(uint y, uint m)
{
	switch (m)
	{
	case  1: return 31;
	case  2: return IsUruu(y) ? 29 : 28;
	case  3: return 31;
	case  4: return 30;
	case  5: return 31;
	case  6: return 30;
	case  7: return 31;
	case  8: return 31;
	case  9: return 30;
	case 10: return 31;
	case 11: return 30;
	case 12: return 31;

	default:
		error();
	}
	return 0; // dummy
}

static void Test_Day2Date(uint64 day, uint ay, uint am, uint ad)
{
	uint ry;
	uint rm;
	uint rd;

	cout("%I64u %04u/%02u/%02u > ", day, ay, am, ad);

	Day2Date(day, &ry, &rm, &rd);

	cout("%04u/%02u/%02u\n", ry, rm, rd);

	errorCase(ry != ay);
	errorCase(rm != am);
	errorCase(rd != ad);
}
static void Test_Date2Day(void)
{
	uint y;
	uint m;
	uint d;
	uint dnum;
	uint64 day = 0;
	uint64 rDay;

	for (y = 1; y <= 3000; y++)
	for (m = 1; m <= 12; m++)
	{
		dnum = GetDayNum(y, m);

		for (d = 1; d <= dnum; d++)
		{
			cout("%04u/%02u/%02u %I64u > ", y, m, d, day);
			rDay = Date2Day(y, m, d);
			cout("%I64u\n", rDay);
			errorCase(day != rDay);

			Test_Day2Date(day, y, m, d);

			day++;
		}
	}
	cout("OK\n");
}
static void Test_Day2Date_02(void)
{
	uint64 day = Date2Day(0xffffffff, 12, 29);

	Test_Day2Date(day + 0, 0xffffffff, 12, 29);
	Test_Day2Date(day + 1, 0xffffffff, 12, 30);
	Test_Day2Date(day + 2, 0xffffffff, 12, 31);
	Test_Day2Date(day + 3, 0xffffffff, 12, 31);
	Test_Day2Date(day + 4, 0xffffffff, 12, 31);

	cout("OK_2\n");
}
static uint CheckDate4Test(uint y, uint m, uint d)
{
	return
		1 <= y &&
		1 <= m && m <= 12 &&
		1 <= d && d <= GetDayNum(y, m);
}
static void Test_CheckDate(void)
{
	uint y;
	uint m;
	uint d;

	for (y = 0; y <= 3000; y++)
	for (m = 0; m <= 13; m++)
	for (d = 0; d <= 32; d++)
	{
		uint tc = CheckDate4Test(y, m, d);
		uint rc = CheckDate(y, m, d);

		tc = tc ? 1 : 0;
		rc = rc ? 1 : 0;

		cout("%04u/%02u/%02u > %u %u\n", y, m, d, tc, rc);

		errorCase(tc != rc);
	}
	cout("OK\n");
}
static void Test_GetEndOfMonth(void)
{
	uint y;
	uint m;
	uint d;

	LOGPOS();

	for (y = 1; y <= 3000; y++)
	for (m = 1; m <= 12; m++)
	{
		uint d = GetEndOfMonth(y, m);

		errorCase(!CheckDate(y, m, d));
		errorCase(CheckDate(y, m, d + 1));

		errorCase(d != GetDayNum(y, m));
	}
	cout("OK\n");
}
int main(int argc, char **argv)
{
	Test_Date2Day();
	Test_Day2Date_02();
	Test_CheckDate();
	Test_GetEndOfMonth();
}
