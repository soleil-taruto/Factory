#include "C:\Factory\Common\all.h"
#include "..\Fraction.h"

static void PrintString(char *str, Fraction_t *i, int printFraction)
{
	if (printFraction)
	{
		char *iStr = Frct_ToFractionLine(i);

		cout("%s = ", iStr);
		memFree(iStr);
	}
	cout("%s\n", str);
}
int main(int argc, char **argv)
{
	uint basement = 10;
	int printFraction = 0;
	char *s1;
	char *s2;
	char *s3;
	Fraction_t *a;
	Fraction_t *b;
	Fraction_t *ans;
	int operator;

readArgs:
	if (argIs("/B"))
	{
		basement = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/F"))
	{
		printFraction = 1;
		goto readArgs;
	}

	while (hasArgs(3))
	{
		s1 = nextArg();
		operator = nextArg()[0];
		s2 = nextArg();

		a = Frct_FromLine(s1);
		b = Frct_FromLine(s2);
		ans = Frct_Calc_xx(a, operator, b);
		s3 = Frct_ToLine(ans, basement);

		PrintString(s3, ans, printFraction);

		memFree(s3);

		while (hasArgs(2))
		{
			operator = nextArg()[0];
			s2 = nextArg();

			a = ans;
			b = Frct_FromLine(s2);
			ans = Frct_Calc_xx(a, operator, b);
			s3 = Frct_ToLine(ans, basement);

			PrintString(s3, ans, printFraction);

			memFree(s3);
		}

		Frct_Release(ans);
	}
}
