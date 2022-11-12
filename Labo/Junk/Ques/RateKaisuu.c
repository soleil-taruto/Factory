/*
	n���1��N���錻�ۂ����ω���̎��s�ŋN���邩�H

	-> n��ۂ��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

static void DoTestOnce(uint n)
{
	uint nu = 0;
	uint de = 0;

	while (de < 1000000)
	{
		uint c;

		for (c = 1; ; c++)
		{
			if (!mt19937_rnd(n))
				break;

			errorCase(UINTMAX <= c);
		}
		errorCase(UINTMAX - nu < c);

		nu += c;
		de++;
	}
	cout("%f = %u / %u\n", (double)nu / de, nu, de);
}
static void DoTest(uint n)
{
	uint c;

	cout("%u���1��N���錻�ۂ����ω���̎��s�ŋN���邩...\n", n);

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

	for (n = 1; n <= 10; n++)
	{
		DoTest(n);
	}
	DoTest(15);
	DoTest(20);
	DoTest(25);
	DoTest(30);
}
