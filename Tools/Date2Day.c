/*
	Date2Day.exe 年

	Date2Day.exe 年 月

	Date2Day.exe 年 月 日

	Date2Day.exe 年 月 日 日数

		日数は sint64

	Date2Day.exe 年a 月a 日a 年b 月b 日b
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Date2Day.h"

static void DispDate(uint y, uint m, uint d)
{
	cout("%04u年%02u月%02u日は\n", y, m, d);
	cout("存在する日付で%s\n", CheckDate(y, m, d) ? "す" : "はありません");
	cout("0001年01月01日の%I64u日後(グレゴリオ暦で換算)\n", Date2Day(y, m, d));
	cout("%s曜日\n", Date2JWeekday(y, m, d));
}
int main(int argc, char **argv)
{
	uint y;
	uint m;
	uint d;
	uint yy;
	uint mm;
	uint dd;
	sint64 day;

	if (hasArgs(6))
	{
		uint64 tmp;
		uint64 tmp2;
		uint64 sum;
		uint64 mid;

		y = toValue(nextArg());
		m = toValue(nextArg());
		d = toValue(nextArg());
		yy = toValue(nextArg());
		mm = toValue(nextArg());
		dd = toValue(nextArg());

		cout("[A]\n");
		DispDate(y, m, d);
		cout("----\n");

		cout("[B]\n");
		DispDate(yy, mm, dd);
		cout("----\n");

		tmp = Date2Day(y, m, d);
		tmp2 = Date2Day(yy, mm, dd);
		sum = tmp + tmp2;
		mid = sum / 2;
		day = (sint64)tmp2 - (sint64)tmp;

		cout("[B]は[A]の%I64d日後です\n", day);

		if (sum % 2 == 1)
		{
			cout("中間の日は２つあります\n");
			cout("----\n");
			cout("★MID-1\n");

			Day2Date(mid, &yy, &mm, &dd);
			DispDate(yy, mm, dd);

			cout("----\n");
			cout("★MID-2\n");

			Day2Date(mid + 1, &yy, &mm, &dd);
			DispDate(yy, mm, dd);
		}
		else
		{
			cout("中間の日は１つあります\n");
			cout("----\n");
			cout("★MID\n");

			Day2Date(mid, &yy, &mm, &dd);
			DispDate(yy, mm, dd);
		}
	}
	if (hasArgs(4))
	{
		y = toValue(nextArg());
		m = toValue(nextArg());
		d = toValue(nextArg());
		day = toInt64(nextArg());

		DispDate(y, m, d);

		cout("----\n");
		cout("%I64d日後は\n", day);

		day += (sint64)Date2Day(y, m, d);

		cout("----\n");

		if (day < 0)
		{
			cout("0001年01月01日の%I64d日前です\n", -day);
		}
		else
		{
			Day2Date((uint64)day, &yy, &mm, &dd);
			DispDate(yy, mm, dd);
		}
		return;
	}
	if (hasArgs(3))
	{
		y = toValue(nextArg());
		m = toValue(nextArg());
		d = toValue(nextArg());

		DispDate(y, m, d);

		return;
	}
	if (hasArgs(2))
	{
		y = toValue(nextArg());
		m = toValue(nextArg());

		cout("%04u年%02u月は\n", y, m);
		cout("日数は%02u日です\n", GetEndOfMonth(y, m));

		return;
	}
	if (hasArgs(1))
	{
		y = toValue(nextArg());

		cout("%04u年は\n", y);
		cout("うるう年で%s\n", GetEndOfMonth(y, 2) == 29 ? "す" : "はありません");

		return;
	}
}
