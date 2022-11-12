/*
	n回に1回起こる現象がn回の試行で1回も起こらない確率

	nが大きくなると -> 1/e
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

static void DoTestOnce(uint n)
{
	uint nu = 0;
	uint de = 0;

	while (de < 100000)
	{
		uint c;

		for (c = 0; c < n; c++)
			if (!mt19937_rnd(n))
				break;

		if (c == n) // ? n回の試行で1回も起こらなかった。
			nu++;

		de++;
	}
	cout("%f = %u / %u\n", (double)nu / de, nu, de);
}
static void DoTest(uint n)
{
	uint c;

	cout("%u回に1回起こる現象が%u回の試行で1回も起こらない確率は...\n", n, n);

	for (c = 1; c <= 10; c++)
	{
		DoTestOnce(n);
	}
	cout("\n");
}
int main(int argc, char **argv)
{
	uint n;

	mt19937_initCRnd();

	for (n = 1; n <= 30; n++)
	{
		DoTest(n);
	}
	for (n = 35; n <= 100; n += 5)
	{
		DoTest(n);
	}
	for (n = 200; n <= 500; n += 50)
	{
		DoTest(n);
	}
}
