#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\autoTable.h"
#include "C:\Factory\Common\Options\Random.h"
#include "C:\Factory\SubTools\libs\bmp.h"

#define OUTPUT_BMPFILE "C:\\Factory\\tmp\\MakeMap.bmp"

static int WipeMode;

static int MarumeCell(autoTable_t *map, int x, int y)
{
	int ctr[2] = { 0, 0 };
	int orig = getTableCell(map, x, y);
	int w = getTableWidth(map);
	int h = getTableHeight(map);
	int xc;
	int yc;

	for (xc = -1; xc <= 1; xc++)
	for (yc = -1; yc <= 1; yc++)
	{
		int sx = x + xc;
		int sy = y + yc;

		if (
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
	return orig != getTableCell(map, x, y);
}
static void MakeMap(int w, int h, int seed, int count)
{
	autoTable_t *map = newTable(getZero, noop_u);
	int x;
	int y;

	errorCase(w < 1);
	errorCase(h < 1);

	mt19937_initRnd(seed);

	resizeTable(map, w, h);

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		setTableCell(map, x, y, mt19937_rnd(w) <= x ? 1 : 0);
	}
	while (0 <= count)
	{
		x = mt19937_rnd(w);
		y = mt19937_rnd(h);

		MarumeCell(map, x, y);

		count--;
	}

	if (WipeMode)
	{
		int c;

		for (c = 10; c; c--)
		{
LOGPOS();
			for (x = 0; x < w; x++)
			for (y = 0; y < h; y++)
			{
				MarumeCell(map, x, y);
			}
LOGPOS();
		}
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
	if (argIs("/W"))
	{
		WipeMode = 1;
	}

	MakeMap(
		toValue(getArg(0)),
		toValue(getArg(1)),
		toValue(getArg(2)),
		toValue(getArg(3))
		);
}
