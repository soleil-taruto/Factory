#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Prime.h"
#include "C:\Factory\Common\Options\Random.h"

static int Test_IsPrime(uint64 value)
{
	uint64 denom;

	if (value < 2)
		return 0;

	if (value == 2)
		return 1;

	if (value % 2 == 0)
		return 0;

	// by C:\Factory\Labo\Proof\Prime64Big.c @ 2015.6.26
	{
		if (value == 18446744073709551557) return 1;
		if (value == 18446744073709551533) return 1;
		if (value == 18446744073709551521) return 1;

		if (UINT64MAX - 100 <= value) return 0;
	}

	for (denom = 3; denom <= UINTMAX; denom += 2)
	{
		if (value < denom * denom)
			break;

		if (value % denom == 0)
			return 0;
	}
	return 1;
}
static void DoTest(uint64 value)
{
	int assume;
	int ret;

	cout("value: %I64u\n", value);

	assume = Test_IsPrime(value);
	cout("assume: %d\n", assume);

	ret = IsPrime(value);
	cout("ret: %d\n", ret);

	errorCase(assume ? !ret : ret);

	cout("OK!\n");
}
int main(int argc, char **argv)
{
	uint64 c;

	mt19937_initRnd(time(NULL));

	for (c = 0; c <= 1000; c++)
	{
		DoTest(c);
	}
	for (c = UINTMAX - 100; c <= (uint64)UINTMAX + 100; c++)
	{
		DoTest(c);
	}
	for (c = UINT64MAX; UINT64MAX - 100 <= c; c--)
	{
		DoTest(c);
	}
	for (c = 0; c < 100000; c++)
	{
		DoTest(mt19937_rnd64Mod((uint64)UINTMAX * 256));
	}
	for (c = 0; c < 10000; c++)
	{
		DoTest(mt19937_rnd64Mod((uint64)UINTMAX * 65536));
	}
	for (c = 0; c < 1000; c++)
	{
		DoTest(mt19937_rnd64Mod((uint64)UINTMAX * 65536 * 256));
	}
	for (c = 0; c < 100; c++)
	{
		DoTest(mt19937_rnd64());
	}
}
