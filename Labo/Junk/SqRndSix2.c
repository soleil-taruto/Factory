#include "C:\Factory\Common\all.h"

static uint GetRand(uint modulo)
{
	return rand() % modulo;
}

#define N 6
#define TEST_NUM 1000000

static uint Sq[N];
static uint SumTbl[N][N];

static void DoTest(uint mode)
{
	uint count;
	uint index;
	uint val;

	zeroclear(SumTbl);

	for (count = 0; count < TEST_NUM; count++)
	{
		for (index = 0; index < N; index++)
			Sq[index] = index;

		for (index = 0; index < N; index++)
		{
			uint r;
			uint tmp = Sq[index];

			if (mode)
				r = GetRand(N);
			else
				r = index + GetRand(N - index);

			Sq[index] = Sq[r];
			Sq[r] = tmp;
		}
		for (index = 0; index < N; index++)
			SumTbl[index][Sq[index]]++;
	}
	cout("%u\n", mode);

	for (index = 0; index < N; index++)
	{
		for (val = 0; val < N; val++)
		{
			if (val)
				cout(", ");

			cout("%f", (double)SumTbl[index][val] / TEST_NUM);
		}
		cout("\n");
	}
	cout("\n");
}
int main(int argc, char **argv)
{
	srand(time(NULL));

	DoTest(0);
	DoTest(1);
}
