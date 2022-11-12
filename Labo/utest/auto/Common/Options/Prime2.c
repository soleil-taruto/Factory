#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Prime2.h"

#define PBIT_P_NUM 3936092160 // from Prime2.c

static void DoTest_One(uint64 value)
{
	int ret1 = IsPrime(value);
	int ret2 = IsPrime_R(value);

	if (pulseSec(2, NULL))
		cout("%I64u -> %d %d\n", value, ret1, ret2);

	if (ret1 ? !ret2 : ret2)
	{
		cout("%I64u -> %d %d ERROR!\n", value, ret1, ret2);
		error();
	}
}
static void DoTest(uint64 minval, uint64 maxval)
{
	uint64 value;

	errorCase(maxval < minval);

	for (value = minval; value < maxval; value++)
	{
		DoTest_One(value);
	}
	DoTest_One(maxval);
}
static void Test_01(void)
{
	DoTest(0, 1000000);
	DoTest(
		(uint64)UINTMAX - 1000000,
		(uint64)UINTMAX + 1000000
		);
	DoTest(
		(uint64)PBIT_P_NUM * 2 - 1000000,
		(uint64)PBIT_P_NUM * 2 + 1000000
		);
	DoTest(
		(UINT64MAX / PBIT_P_NUM - 1) * PBIT_P_NUM - 100,
		(UINT64MAX / PBIT_P_NUM - 1) * PBIT_P_NUM + 100
		);
	DoTest(
		(UINT64MAX / PBIT_P_NUM) * PBIT_P_NUM - 100,
		(UINT64MAX / PBIT_P_NUM) * PBIT_P_NUM + 100
		);
	DoTest(
		UINT64MAX - 100,
		UINT64MAX
		);

	// ----

	DoTest(
		UINT64MAX - 30100,
		UINT64MAX - 30000
		);
	DoTest(
		UINT64MAX - 20100,
		UINT64MAX - 20000
		);
	DoTest(
		UINT64MAX - 10100,
		UINT64MAX - 10000
		);

	// ----

	cout("OK!\n");
}
int main(int argc, char **argv)
{
	Test_01();
}
