/*
	Prime.exe (/F value | /FF value | /C minval maxval | [/P] (value | minval maxval))
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Prime.h"

static int PrintPrimeOnlyFlag;

static void PrintIsPrime(uint64 value)
{
	if (PrintPrimeOnlyFlag)
	{
		if (IsPrime(value))
			cout("%I64u\n", value);
	}
	else
	{
		cout("%I64u %s prime.\n", value, IsPrime(value) ? "is" : "is not");
	}
}
int main(int argc, char **argv)
{
	if (argIs("/F"))
	{
		uint64 value = toValue64(nextArg());
		uint64 factors[64];
		uint index;

		Factorization(value, factors);

		cout("%I64u factors are:\n", value);

		for (index = 0; factors[index] != 0; index++)
			cout("%I64u\n", factors[index]);

		return;
	}
	if (argIs("/FF"))
	{
		uint64 value = toValue64(nextArg());
		uint64 factors[64];
		uint index;

		Factorization(value, factors);

		cout("%I64u factors are:\n", value);

		for (index = 0; factors[index] != 0; )
		{
			uint x;

			for (x = 1; factors[index] == factors[index + x]; x++);

			cout("%I64u\t%u\n", factors[index], x);

			index += x;
		}
		return;
	}
	if (argIs("/C"))
	{
		uint64 minval;
		uint64 maxval;
		uint64 value;
		uint64 count = 0;

		minval = toValue64(nextArg());
		maxval = toValue64(nextArg());

		if (maxval == 0)
			maxval = UINT64MAX;

		errorCase(maxval < minval);

		for (value = minval; ; value++)
		{
			if (IsPrime(value))
				count++;

			if (value == maxval)
				break;
		}
		cout("%I64u\n", count);
		return;
	}
	if (argIs("/P"))
	{
		PrintPrimeOnlyFlag = 1;
	}
	if (hasArgs(2))
	{
		uint64 minval;
		uint64 maxval;
		uint64 value;

		minval = toValue64(nextArg());
		maxval = toValue64(nextArg());

		if (maxval == 0)
			maxval = UINT64MAX;

		errorCase(maxval < minval);

		for (value = minval; ; value++)
		{
			if (eqIntPulseSec(2, NULL))
			{
				int cancelled = 0;

				while (hasKey())
					if (getKey() == 0x1b)
						cancelled = 1;

				if (cancelled)
				{
					cout("cancelled!\n");
					break;
				}
			}
			PrintIsPrime(value);

			if (value == maxval)
				break;
		}
		return;
	}

	PrintIsPrime(toValue64(nextArg()));
}
