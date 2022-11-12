#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\autoTable.h"
#include "C:\Factory\Common\Options\Random.h"
#include "C:\Factory\SubTools\libs\bmp.h"

#define OUTPUT_BMPFILE "C:\\Factory\\tmp\\MakeMap.bmp"

static int LoopMode;

static void MakeMap(int w, int h, int seed, int permil, int count)
{
	autoTable_t *map = newTable(getZero, noop_u);
	int x;
	int y;

	errorCase(w < 1);
	errorCase(h < 1);
	errorCase(permil < 0 || 1000 < permil);

	mt19937_initRnd(seed);

	resizeTable(map, w, h);

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		setTableCell(map, x, y, mt19937_rnd(1000) < permil ? 1 : 0);
	}
	while (0 <= count)
	{
		int ctr[2] = { 0, 0 };
		int xc;
		int yc;

		x = mt19937_rnd(w);
		y = mt19937_rnd(h);

		for (xc = -1; xc <= 1; xc++)
		for (yc = -1; yc <= 1; yc++)
		{
			int sx = x + xc;
			int sy = y + yc;

			if (
				LoopMode ||
				0 <= sx && sx < w &&
				0 <= sy && sy < h
				)
			{
				sx = (sx + w) % w;
				sy = (sy + h) % h;

				ctr[getTableCell(map, sx, sy)]++;
			}
		}
		if (ctr[0] != ctr[1])
		{
			setTableCell(map, x, y, ctr[0] < ctr[1] ? 1 : 0);
		}
		count--;
	}

	if (LoopMode)
	{
		resizeTable(map, w * 3, h * 3);

		for (x = 0; x < w; x++)
		for (y = 0; y < h; y++)
		{
			int xc;
			int yc;

			for (xc = 0; xc < 3; xc++)
			for (yc = 0; yc < 3; yc++)
			{
				if (xc || yc)
					setTableCell(map, w * xc + x, h * yc + y, getTableCell(map, x, y));
			}
		}

		w *= 3;
		h *= 3;
	}

	{
		autoList_t *bmp = newList();

		for (y = 0; y < h; y++)
		{
			autoList_t *row = newList();

			for (x = 0; x < w; x++)
			{
				addElement(row, getTableCell(map, x, y) ? 0x445599 : 0xaabbff);
			}
			addElement(bmp, (uint)row);
		}
		writeBMPFile(OUTPUT_BMPFILE, bmp);
	}
	execute("START " OUTPUT_BMPFILE);
}
int main(int argc, char **argv)
{
	if (argIs("/L"))
	{
		LoopMode = 1;
	}

	MakeMap(
		toValue(getArg(0)),
		toValue(getArg(1)),
		toValue(getArg(2)),
		toValue(getArg(3)),
		toValue(getArg(4))
		);
}
