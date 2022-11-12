#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

#define SUGO_LEN 30

int main(int argc, char **argv)
{
	uint sugo_n[SUGO_LEN];
	uint d;
	uint c;

	mt19937_initCRnd();

	memset(sugo_n, 0x00, sizeof(sugo_n));
	d = 0;

	while (d < 10000000)
	{
		c = 0;

		while (c < SUGO_LEN)
		{
			sugo_n[c]++;

			c += mt19937_range(1, 6);
		}
		d++;
	}

	for (c = 0; c < SUGO_LEN; c++)
	{
		cout("[%02u] %f = %u / %u\n", c, (double)sugo_n[c] / d, sugo_n[c], d);
	}
}
