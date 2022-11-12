/*
	n 個の的に n 回投げて何パーセントに当たるか。
	-> 0.632 くらい -> (1 - 1/e) ぽい。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"

static void DoTest(uint n)
{
	uchar *mato = memCalloc(n);
	uint c;
	uint numer = 0;
	uint denom = n;

	for (c = 0; c < n; c++)
	{
		mato[mt19937_rnd(n)] = 1;
	}
	for (c = 0; c < n; c++)
	{
		if (mato[c])
		{
			numer++;
		}
	}
	memFree(mato);
	cout("[%u] %u / %u = %f\n", n, numer, denom, (double)numer / denom);
}
int main(int argc, char **argv)
{
	uint n;
	uint c;

	mt19937_initRnd(time(NULL));

	for (n = 10; n <= 1000000; n *= 10)
	for (c = 0; c < 10; c++)
	{
		DoTest(n);
	}
}
