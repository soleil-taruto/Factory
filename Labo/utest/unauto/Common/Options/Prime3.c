#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"
#include "C:\Factory\Common\Options\Progress.h"
#include "C:\Factory\Common\Options\Prime.h"
#include "C:\Factory\Common\Options\Prime3.h"

static void Test_01(void)
{
	error(); // dummy func
}

static void Test_02b(uint64 value)
{
	int a;
	int a2;

	cout("test value: %I64u\n", value);
	a = IsPrime(value);
	cout("is prime: %d\n", a);
	a2 = IsPrime_M(value);

	errorCase(a ? !a2 : a2);
}
static void Test_02(void)
{
	uint64 v;

	for (v = 0; v <= 10000; v++)
	{
		Test_02b(v);
	}
	for (v = 0; v <= 1000000; v += getCryptoRand64() % 200 + 1)
	{
		Test_02b(v);
	}
	for (v = 0; v <= 10000000000; v += getCryptoRand64() % 2000000 + 1)
	{
		Test_02b(v);
	}
	for (v = 0; v <= 10000000000000000; v += getCryptoRand64() % 2000000000000 + 1)
	{
		Test_02b(v);
	}
	for (v = UINT64MAX; UINT64MAX - 100 < v; v--)
	{
		Test_02b(v);
	}
	for (v = UINT64MAX; UINT64MAX - 10000 < v; v -= getCryptoRand64() % 200 + 1)
	{
		Test_02b(v);
	}
	for (v = UINT64MAX; UINT64MAX - 100000000 < v; v -= getCryptoRand64() % 2000000 + 1)
	{
		Test_02b(v);
	}
	for (v = UINT64MAX; UINT64MAX - 100000000000000 < v; v -= getCryptoRand64() % 2000000000000 + 1)
	{
		Test_02b(v);
	}
	for (; ; )
	{
		Test_02b(getCryptoRand64());
	}
}

int main(int argc, char **argv)
{
//	Test_01();
	Test_02();
}
