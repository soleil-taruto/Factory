/*
	すごろくで十分遠いマスに止まる確率 ... 2/7 ?
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\mt19937.h"

static int TestOnce(int target)
{
	int pos = 0;

	while (pos < target)
	{
		pos += mt19937_rnd(6) + 1;
	}
	return pos == target;
}
static int TestOnce_VeryFar(void)
{
	return TestOnce(mt19937_rnd(10000) + 10000);
}
int main(int argc, char **argv)
{
	int d;

	mt19937_init();

#if 0
	for (d = 0; d < 10; d++)
	{
		int testcnt = 300000;
		int enactcnt = 0;
		int c;

		for (c = 0; c < testcnt; c++)
		{
			if (c % 1000 == 0)
				cmdTitle_x(xcout("Sugoroku - %d", c));

			if (TestOnce_VeryFar())
				enactcnt++;
		}
		cout("%d / %d = %f -> * 7 = %f\n"
			,enactcnt
			,testcnt
			,(double)enactcnt / testcnt
			,((double)enactcnt / testcnt) * 7
			);
	}
#else
	for (d = 0; d < 30; d++)
	{
		int testcnt = 300000;
		int enactcnt = 0;
		int c;

		for (c = 0; c < testcnt; c++)
		{
			if (c % 1000 == 0)
				cmdTitle_x(xcout("Sugoroku - %d", c));

			if (TestOnce(d))
				enactcnt++;
		}
		cout("%2d: %d / %d = %f\n"
			,d
			,enactcnt
			,testcnt
			,(double)enactcnt / testcnt
			);
	}
#endif
}
