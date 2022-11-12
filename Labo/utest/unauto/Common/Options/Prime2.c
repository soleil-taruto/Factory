#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Prime2.h"

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
static void DoTest_Rev(uint64 minval, uint64 maxval)
{
	uint64 value;

	errorCase(maxval < minval);

	for (value = maxval; minval < value; value--)
	{
		DoTest_One(value);
	}
	DoTest_One(minval);
}
static void Test_01(void)
{
	DoTest_Rev(18446744000000000000, UINT64MAX);

	cout("OK!\n");
}
int main(int argc, char **argv)
{
	Test_01();
}
