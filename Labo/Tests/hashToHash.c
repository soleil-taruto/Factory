/*
	ハッシュ値のハッシュ値を取り続けたら、ハッシュ値の取り得る値が少なくなるんじゃないかという懸念。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"

#define MAPMAX 1000000
#define W 300
#define H 300

int main(int argc, char **argv)
{
	uchar *map = (uchar *)memAlloc(MAPMAX);
	uint c;
	uint r;

	mt19937_initRnd((uint)time(NULL));

	for (r = H; r; r--)
	{
		uint m = mt19937_rnd(MAPMAX) + 1;
		uint t;
		uint i;
		uint d;

		cmdTitle_x(xcout("hashToHash - %u", r));
		t = m;

		for (c = W; c; c--)
		{
			memset(map, 0x00, m);

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

			t = d;
		}
		cout("\n");
	}
	cmdTitle("hashToHash");
	memFree(map);
}
