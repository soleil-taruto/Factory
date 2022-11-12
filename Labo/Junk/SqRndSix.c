#include "C:\Factory\Common\all.h"

#define N 6

static uint Rnds[N];
static uint Sq[N];
static uint SumTbl[2][N][N];
static uint Totals[2];

static void DoShuffle(void)
{
	uint index;
	uint mode;

	for (index = 0; index < N; index++)
		Sq[index] = index;

	for (index = 0; index < N; index++)
	{
		uint r = Rnds[index];
		uint tmp = Sq[index];

		Sq[index] = Sq[r];
		Sq[r] = tmp;
	}
	for (index = 0; index < N; index++)
		if (Rnds[index] < index)
			break;

	for (mode = index == N ? 0 : 1; mode < 2; mode++)
	{
		for (index = 0; index < N; index++)
			SumTbl[mode][index][Sq[index]]++;

		Totals[mode]++;
	}
}
static void ShowResult(uint mode, int rateMode)
{
	uint index;
	uint val;

	cout("%u\n", mode);

	for (index = 0; index < N; index++)
	{
		for (val = 0; val < N; val++)
		{
			if (val)
				cout(", ");

			if (!rateMode)
				cout("%u", SumTbl[mode][index][val]);
			else
				cout("%f", (double)SumTbl[mode][index][val] / Totals[mode]);
		}
		cout("\n");
	}
	if (!rateMode)
		cout("%u\n", Totals[mode]);

	cout("\n");
}
static void DoTest(void)
{
	uint index;

	for (; ; )
	{
		DoShuffle();

		for (index = 0; index < N; index++)
		{
			if (Rnds[index] + 1 < N)
			{
				Rnds[index]++;
				break;
			}
			Rnds[index] = 0;
		}
		if (index == N)
			break;
	}
	ShowResult(0, 0);
	ShowResult(1, 0);
	ShowResult(0, 1);
	ShowResult(1, 1);
}
int main(int argc, char **argv)
{
	DoTest();
}
