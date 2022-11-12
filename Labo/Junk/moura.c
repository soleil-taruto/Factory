#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"

static double Moura(uint num, double rate)
{
	autoBlock_t *map = newBlock();
	uint index;
	uint end = m_d2i(num * rate);
	uint count = 0;
	double ansRate;

	setSize(map, num);

	for (index = 0; index < end; index++)
		setByte(map, mt19937_rnd(num), 1);

	for (index = 0; index < num; index++)
		if (getByte(map, index))
			count++;

	ansRate = (double)count / num;
	return ansRate;
}
static void MouraTest(uint num, double rate)
{
	cout("num=%u, rate=%f, ans=%f\n", num, rate, Moura(num, rate));
}
int main(int argc, char **argv)
{
	if (argIs("/1"))
	{
#if 1
		MouraTest(10000000, 1.0); // 1/e
		MouraTest(10000000, 10.0);
		MouraTest(10000000, 20.0);
		MouraTest(10000000, 30.0);
//		MouraTest(10000000, 64.0);
#else
		MouraTest(1000000, 1.0);
		MouraTest(1000000, 2.0);
		MouraTest(1000000, 3.0);
		MouraTest(1000000, 10.0);
		MouraTest(1000000, 11.0);
		MouraTest(1000000, 12.0);
		MouraTest(1000000, 62.0);
		MouraTest(1000000, 63.0);
		MouraTest(1000000, 64.0);
#endif
		return;
	}

	{
		uint num;

		for (num = 10; num <= 10000; num *= 10)
		{
			uint numer;
			uint denom = 10;

			for (numer = denom; numer < denom * 70; numer++)
			{
				MouraTest(num, (double)numer / denom);
			}
		}
	}
}
