/*
	(UINT64MAX + 1) % modulo
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Calc.h"
#include "C:\Factory\Common\Options\CRRandom.h"

#define S_2P64 "18446744073709551616" // 2 ^ 64

static uint64 DoTest_01_a(uint64 modulo) // ŒŸØ—p
{
	char *v = xcout("%I64u", modulo);
	char *d;
	char *m;
	char *s;
	uint64 ret;

	d = calcLine(S_2P64, '/', v, 10, 0);
	m = calcLine(d,      '*', v, 10, 0);
	s = calcLine(S_2P64, '-', m, 10, 0);

	ret = toValue64(s);

	memFree(v);
	memFree(d);
	memFree(m);
	memFree(s);

	return ret;
}
static uint64 DoTest_01_b(uint64 modulo) // ŠÌ‚ñ‚Æ‚±
{
	return ((UINT64MAX % modulo) + 1) % modulo;
}
static void DoTest_01(uint64 modulo)
{
	uint64 a = DoTest_01_a(modulo);
	uint64 b = DoTest_01_b(modulo);

	cout("%I64u -> %I64u %I64u\n", modulo, a, b);

	errorCase(a != b);
}
static void DoTest(void)
{
	uint64 count;

	for (count = 1; count <= 10000; count++)
	{
		DoTest_01(count);
	}
	for (count = SINT16MAX - 10000; count <= (uint64)SINT16MAX + 10000; count++)
	{
		DoTest_01(count);
	}
	for (count = UINT16MAX - 10000; count <= (uint64)UINT16MAX + 10000; count++)
	{
		DoTest_01(count);
	}
	for (count = SINTMAX - 10000; count <= (uint64)SINTMAX + 10000; count++)
	{
		DoTest_01(count);
	}
	for (count = UINTMAX - 10000; count <= (uint64)UINTMAX + 10000; count++)
	{
		DoTest_01(count);
	}
	for (count = SINT64MAX - 10000; count <= SINT64MAX + 10000; count++)
	{
		DoTest_01(count);
	}
	for (count = UINT64MAX - 10000; count < UINT64MAX; count++)
	{
		DoTest_01(count);
	}
	DoTest_01(UINT64MAX);

	while (waitKey(0) != 0x1b)
	{
		DoTest_01(mt19937_rnd64() & SINT64MAX);
	}
}
int main(int argc, char **argv)
{
	mt19937_initCRnd();

	DoTest();
}
