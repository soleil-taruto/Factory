#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"
#include "C:\Factory\Common\Options\Progress.h"
#include "C:\Factory\Common\Options\Prime.h"
#include "C:\Factory\Common\Options\Prime3_v1.h"

#define TEST_K 20

static void Test_01(void)
{
	uint correctCount = 0;
	uint incorrectCount = 0;
	uint primeCount = 0;
	uint notPrimeCount = 0;

	for (; ; )
	{
		uint64 value = getCryptoRand64();
		int a1;
		int a2;

		a1 = IsPrime(value);
		a2 = IsPrime_M_K(value, TEST_K);

		if (a1 ? a2 : !a2)
			correctCount++;
		else
			incorrectCount++;

		if (a1)
			primeCount++;
		else
			notPrimeCount++;

		cout("correct: %u, incorrect: %u, prime: %u, not-prime: %u\n", correctCount, incorrectCount, primeCount, notPrimeCount);
	}
}

//#define CHECK_COUNT 30
#define CHECK_COUNT 100

static void Test_02b(uint64 value)
{
	int a;
	uint c;

	cout("test value: %I64u\n", value);
	a = IsPrime(value);
	cout("is prime: %d\n", a);

	ProgressBegin();

	for (c = 0; c < CHECK_COUNT; c++)
	{
		ProgressRate(c * 1.0 / CHECK_COUNT);

		if (IsPrime_M_K(value, TEST_K) ? a : !a)
		{
			// ok
		}
		else
		{
			error(); // bugged !!! or 不幸な事故？？？
		}
	}
	ProgressEnd(0);

	cout("OK!\n");
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
