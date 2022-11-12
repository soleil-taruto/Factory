#include "C:\Factory\Common\all.h"

#define SUM_MAX 100

static uint PtnNum;

static void TryNext(uint startNum, uint currSum)
{
	uint count;

	if (currSum == SUM_MAX)
	{
		PtnNum++;
		return;
	}

	for (count = startNum; currSum + count <= SUM_MAX; count++)
	{
		TryNext(count + 1, currSum + count);
	}
}
int main(int argc, char **argv)
{
	TryNext(1, 0);

	cout("%u\n", PtnNum);
}
