#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"
#include "C:\Factory\Common\Options\Calc2.h"
#include "C:\Factory\Common\Options\uintx\uint4096.h"

static char *TrimValueString(char *str)
{
	char *p;

	if (!*str)
		return addChar(str, '0');

	for (p = str; *p == '0' && p[1]; )
		p++;

	copyLine(str, p);

	return str;
}
static char *MakeTestHexValue_Scale(uint scale)
{
	autoBlock_t *buff = newBlock();
	uint index;

	for (index = 0; index < scale; index++)
		addByte(buff, hexadecimal[mt19937_rnd(16)]);

	return TrimValueString(unbindBlock2Line(buff));
}
static char *MakeTestHexValue(void)
{
	uint scale;

	if (mt19937_rnd(4)) // 75%
	{
		scale = mt19937_range(0, 1024);
	}
	else // 25%
	{
		switch (mt19937_rnd(2))
		{
		case 0: scale = mt19937_range(0, 10); break;
		case 1: scale = mt19937_range(1014, 1024); break;

		default:
			error();
		}
	}
	return MakeTestHexValue_Scale(scale);
}
static UI4096_t FromString(char *str)
{
	uint arr[128] = { 0 };
	uint index;
	uint sLen = strlen(str);

	for (index = 0; index < sLen; index++)
		arr[index / 8] |= m_c2i(str[sLen - 1 - index]) << index % 8 * 4;

	return ToUI4096(arr);
}
static char *ToString(UI4096_t value)
{
	autoBlock_t *buff = newBlock();
	uint arr[128];
	uint index;

	FromUI4096(value, arr);

	for (index = 128; index; index--)
		ab_addLine_x(buff, xcout("%08x", arr[index - 1]));

	return TrimValueString(unbindBlock2Line(buff));
}

#define H_4096 "10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

static void Test01(void)
{
	calcRadix = 16;
	calcBasement = 0;

	while (!waitKey(0))
	{
		// ADD
		{
			char *s1 = MakeTestHexValue();
			char *s2 = MakeTestHexValue();
			char *sa;
			UI4096_t t1;
			UI4096_t t2;
			UI4096_t ta;
			char *t1s;
			char *t2s;
			char *tas;

			sa = calc(s1, '+', s2);

			if (1024 < strlen(sa))
			{
				eraseChar(sa);
				sa = TrimValueString(sa);
			}
			t1 = FromString(s1);
			t2 = FromString(s2);

			ta = UI4096_Add(t1, t2, NULL);

			t1s = ToString(t1);
			t2s = ToString(t2);
			tas = ToString(ta);

			cout("[ADD]\n");
			cout("s1: %s\n", s1);
			cout("s2: %s\n", s2);
			cout("sa: %s\n", sa);
			cout("t1: %s\n", t1s);
			cout("t2: %s\n", t2s);
			cout("ta: %s\n", tas);

			errorCase(strcmp(s1, t1s));
			errorCase(strcmp(s2, t2s));
			errorCase(strcmp(sa, tas));

			memFree(s1);
			memFree(s2);
			memFree(sa);
			memFree(t1s);
			memFree(t2s);
			memFree(tas);
		}

		// SUB
		{
			char *s1 = MakeTestHexValue();
			char *s2 = MakeTestHexValue();
			char *sa;
			UI4096_t t1;
			UI4096_t t2;
			UI4096_t ta;
			char *t1s;
			char *t2s;
			char *tas;

			sa = calc(s1, '-', s2);

			if (sa[0] == '-')
				sa = calc(sa, '+', H_4096);

			t1 = FromString(s1);
			t2 = FromString(s2);

			ta = UI4096_Sub(t1, t2);

			t1s = ToString(t1);
			t2s = ToString(t2);
			tas = ToString(ta);

			cout("[SUB]\n");
			cout("s1: %s\n", s1);
			cout("s2: %s\n", s2);
			cout("sa: %s\n", sa);
			cout("t1: %s\n", t1s);
			cout("t2: %s\n", t2s);
			cout("ta: %s\n", tas);

			errorCase(strcmp(s1, t1s));
			errorCase(strcmp(s2, t2s));
			errorCase(strcmp(sa, tas));

			memFree(s1);
			memFree(s2);
			memFree(sa);
			memFree(t1s);
			memFree(t2s);
			memFree(tas);
		}

		// MUL
		{
			char *s1 = MakeTestHexValue();
			char *s2 = MakeTestHexValue();
			char *sa;
			UI4096_t t1;
			UI4096_t t2;
			UI4096_t ta;
			char *t1s;
			char *t2s;
			char *tas;

			sa = calc(s1, '*', s2);

			if (1024 < strlen(sa))
			{
				eraseLine(sa, strlen(sa) - 1024);
				sa = TrimValueString(sa);
			}
			t1 = FromString(s1);
			t2 = FromString(s2);

			ta = UI4096_Mul(t1, t2, NULL);

			t1s = ToString(t1);
			t2s = ToString(t2);
			tas = ToString(ta);

			cout("[MUL]\n");
			cout("s1: %s\n", s1);
			cout("s2: %s\n", s2);
			cout("sa: %s\n", sa);
			cout("t1: %s\n", t1s);
			cout("t2: %s\n", t2s);
			cout("ta: %s\n", tas);

			errorCase(strcmp(s1, t1s));
			errorCase(strcmp(s2, t2s));
			errorCase(strcmp(sa, tas));

			memFree(s1);
			memFree(s2);
			memFree(sa);
			memFree(t1s);
			memFree(t2s);
			memFree(tas);
		}

		// DIV
		{
			char *s1 = MakeTestHexValue();
			char *s2 = MakeTestHexValue();
			char *sa;
			UI4096_t t1;
			UI4096_t t2;
			UI4096_t ta;
			char *t1s;
			char *t2s;
			char *tas;

			if (!strcmp(s2, "0"))
				*s2 = '1';

			LOGPOS();
			sa = calc(s1, '/', s2);
			LOGPOS();

			t1 = FromString(s1);
			t2 = FromString(s2);

			LOGPOS();
			ta = UI4096_Div(t1, t2, NULL);
			LOGPOS();

			t1s = ToString(t1);
			t2s = ToString(t2);
			tas = ToString(ta);

			cout("[DIV]\n");
			cout("s1: %s\n", s1);
			cout("s2: %s\n", s2);
			cout("sa: %s\n", sa);
			cout("t1: %s\n", t1s);
			cout("t2: %s\n", t2s);
			cout("ta: %s\n", tas);

			errorCase(strcmp(s1, t1s));
			errorCase(strcmp(s2, t2s));
			errorCase(strcmp(sa, tas));

			memFree(s1);
			memFree(s2);
			memFree(sa);
			memFree(t1s);
			memFree(t2s);
			memFree(tas);
		}
	}
	cout("Stopped\n");
}
int main(int argc, char **argv)
{
	mt19937_initCRnd();

	Test01();
}
