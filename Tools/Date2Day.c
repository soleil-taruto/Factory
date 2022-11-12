/*
	Date2Day.exe �N

	Date2Day.exe �N ��

	Date2Day.exe �N �� ��

	Date2Day.exe �N �� �� ����

		������ sint64

	Date2Day.exe �Na ��a ��a �Nb ��b ��b
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Date2Day.h"

static void DispDate(uint y, uint m, uint d)
{
	cout("%04u�N%02u��%02u����\n", y, m, d);
	cout("���݂�����t��%s\n", CheckDate(y, m, d) ? "��" : "�͂���܂���");
	cout("0001�N01��01����%I64u����(�O���S���I��Ŋ��Z)\n", Date2Day(y, m, d));
	cout("%s�j��\n", Date2JWeekday(y, m, d));
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

		cout("[B]��[A]��%I64d����ł�\n", day);

		if (sum % 2 == 1)
		{
			cout("���Ԃ̓��͂Q����܂�\n");
			cout("----\n");
			cout("��MID-1\n");

			Day2Date(mid, &yy, &mm, &dd);
			DispDate(yy, mm, dd);

			cout("----\n");
			cout("��MID-2\n");

			Day2Date(mid + 1, &yy, &mm, &dd);
			DispDate(yy, mm, dd);
		}
		else
		{
			cout("���Ԃ̓��͂P����܂�\n");
			cout("----\n");
			cout("��MID\n");

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
		cout("%I64d�����\n", day);

		day += (sint64)Date2Day(y, m, d);

		cout("----\n");

		if (day < 0)
		{
			cout("0001�N01��01����%I64d���O�ł�\n", -day);
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

		cout("%04u�N%02u����\n", y, m);
		cout("������%02u���ł�\n", GetEndOfMonth(y, m));

		return;
	}
	if (hasArgs(1))
	{
		y = toValue(nextArg());

		cout("%04u�N��\n", y);
		cout("���邤�N��%s\n", GetEndOfMonth(y, 2) == 29 ? "��" : "�͂���܂���");

		return;
	}
}
