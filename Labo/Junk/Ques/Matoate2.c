/*
	hash(x), hash(hash(x)), hash(hash(hash(x))) ... ÇƒÇ‚Ç¡ÇƒÇ¢ÇØÇŒé˚ë©Ç∑ÇÈÇÒÇ∂Ç·Ç»Ç¢Ç©Ç∆ÅB
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"

static FILE *Fp;
static FILE *FpN;

static void DoTest(uint n, uint testNum)
{
	uint *mato = (uint *)memAlloc(n * sizeof(uint));
	uchar *inha = (uchar *)memAlloc(n);
	uint c;
	uint i;
	uint numer;
	uint denom;

	writeToken_x(Fp, xcout("%u,", n));
	writeToken_x(FpN, xcout("%u,", n));

	for (c = 0; c < n; c++)
	{
		mato[c] = mt19937_rnd(n);
		inha[c] = 1;
	}
	for (i = 0; i < testNum; i++)
	{
		for (c = 0; c < n; c++)
		{
			if (inha[c] & 1)
			{
				inha[mato[c]] |= 2;
			}
		}
		for (c = 0; c < n; c++)
		{
			inha[c] >>= 1;
		}
		numer = 0;
		denom = n;

		for (c = 0; c < n; c++)
		{
			if (inha[c])
			{
				numer++;
			}
		}
		cout("%u [%u] %u / %u = %f\n", n, i, numer, denom, (double)numer / denom);

		writeToken_x(Fp, xcout("%f,", (double)numer / denom));
		writeToken_x(FpN, xcout("%u,", numer));
	}
	writeLine(Fp, "");
	writeLine(FpN, "");

	memFree(mato);
	memFree(inha);
}
int main(int argc, char **argv)
{
	uint n;
	uint c;

	mt19937_initRnd(time(NULL));

	Fp = fileOpen(getOutFile("rate.csv"), "wt");
	FpN = fileOpen(getOutFile("numer.csv"), "wt");

	for (n = 10; n <= 1000000; n *= 10)
	for (c = 0; c < 10; c++)
	{
		DoTest(n, 1000);
	}
	fileClose(Fp);
	fileClose(FpN);
	Fp = NULL;
	FpN = NULL;

	openOutDir();
}
