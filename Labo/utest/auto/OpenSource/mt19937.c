#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\mt19937.h"

static void Test_mt19937_range_Range(uint minval, uint maxval)
{
	uint count;
	uint trycnt;
	uint r;

	cout("Test_mt19937_range_Range: %u, %u\n", minval, maxval);

	for (count = 1000; count; count--)
	{
		r = mt19937_range(minval, maxval);

		errorCase(r < minval);
		errorCase(maxval < r);
	}
	for (count = minval; count <= maxval; count++)
	{
		for (trycnt = 1; ; trycnt++)
		{
			r = mt19937_range(minval, maxval);

			errorCase(r < minval);
			errorCase(maxval < r);

			if (r == count)
			{
				break;
			}
			errorCase(1000000 < trycnt); // 非常に低い確率だが正常でもエラーになる。
		}
	}
	cout("OK\n");
}
static void Test_mt19937_range(void)
{
	uint minval;
	uint maxval;

#define VALBEGIN 0
#define VALEND 100

	for (minval = VALBEGIN; minval <= VALEND; minval++)
	for (maxval = minval;   maxval <= VALEND; maxval++)
	{
		Test_mt19937_range_Range(minval, maxval);
	}
}
int main(int argc, char **argv)
{
	mt19937_init();

	Test_mt19937_range();
}
