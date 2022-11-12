/*
	ハッシュ値の何倍の大きさの乱数なら、取り得る値（ほぼ）全て使えるか。

	----

	x  1.0 -> 0.633
	x  2.0 -> 0.835
	x  3.0 -> 0.952
	x  4.0 -> 0.981
	x  5.0 -> 0.993
	x  6.0 -> 0.998
	x  7.0 -> 0.9991
	x  8.0 -> 0.9997 ... CrtptoRand() は 4096 バイトなのでここ
	x  9.0 -> 0.99992
	x 10.0 -> 0.99997
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"

#define MAPMAX 10000
#define W 100
#define H 100

#define I2Rate(i) \
	((i + 1) * 0.2)

int main(int argc, char **argv)
{
	uchar *map = (uchar *)memAlloc(MAPMAX);
	uint c;
	uint r;

	mt19937_initRnd((uint)time(NULL));

	for (c = 0; c < W; c++)
	{
		cout("%f,", I2Rate(c));
	}
	cout("\n");

	for (r = 0; r < H; r++)
	{
		for (c = 0; c < W; c++)
		{
			uint m = mt19937_rnd(MAPMAX) + 1;
			uint t;
			uint i;
			uint d;

			memset(map, 0x00, m);

			t = m_d2i(m * I2Rate(c));

			for (i = 0; i < t; i++)
			{
				map[mt19937_rnd(m)] = 1;
			}
			d = 0;

			for (i = 0; i < m; i++)
			{
				if (map[i]) d++;
			}
			cout("%f,", d / (double)m);
		}
		cout("\n");
	}
	memFree(map);
}
