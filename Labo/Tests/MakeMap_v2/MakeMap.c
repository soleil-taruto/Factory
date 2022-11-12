#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\autoTable.h"
#include "C:\Factory\Common\Options\Random.h"
#include "C:\Factory\SubTools\libs\bmp.h"

#define OUTPUT_BMPFILE "C:\\Factory\\tmp\\MakeMap.bmp"
#define COLOR_0 0xaabbff
#define COLOR_1 0x445599

static autoTable_t *Map;

static void LoadMap(void)
{
	autoList_t *bmp = readBMPFile(OUTPUT_BMPFILE);
	int x;
	int y;
	int w;
	int h;

	w = getCount(getList(bmp, 0));
	h = getCount(bmp);

	resizeTable(Map, w, h);

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		int color = getElement(getList(bmp, y), x);
		int flag;

		switch (color)
		{
		case COLOR_0: flag = 0; break;
		case COLOR_1: flag = 1; break;

		default:
			error();
		}
		setTableCell(Map, x, y, flag);
	}
	releaseDim_BR(bmp, 2, NULL);
}
static void SaveMap(void)
{
	autoList_t *bmp = newList();
	int x;
	int y;
	int w = getTableWidth(Map);
	int h = getTableHeight(Map);

	for (y = 0; y < h; y++)
	{
		autoList_t *row = newList();

		for (x = 0; x < w; x++)
		{
			addElement(row, getTableCell(Map, x, y) ? COLOR_1 : COLOR_0);
		}
		addElement(bmp, (uint)row);
	}
	writeBMPFile_cx(OUTPUT_BMPFILE, bmp);
}

static void InitMap(int w, int h, int flag)
{
	int x, y;

	errorCase(w < 1);
	errorCase(h < 1);
	flag = flag ? 1 : 0;

	resizeTable(Map, w, h);

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		setTableCell(Map, x, y, flag);
	}
	SaveMap();
}
static void ExpandMap(int expand)
{
	int x, y, w, h;

	LoadMap();

	w = getTableWidth(Map);
	h = getTableHeight(Map);

	errorCase(expand < 1);

	w *= expand;
	h *= expand;

	resizeTable(Map, w, h);

	for (x = w - 1; 0 <= x; x--)
	for (y = h - 1; 0 <= y; y--)
	{
		setTableCell(Map, x, y, getTableCell(Map, x / expand, y / expand));
	}
	SaveMap();
}
static void RandomizeMap(int permil_0, int permil_1)
{
	int x, y, w, h;

	LoadMap();

	w = getTableWidth(Map);
	h = getTableHeight(Map);

	errorCase(permil_0 < 0 || 1000 < permil_0);
	errorCase(permil_1 < 0 || 1000 < permil_1);

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		setTableCell(
			Map,
			x,
			y,
			mt19937_rnd(1000) < (getTableCell(Map, x, y) ? permil_1: permil_0) ? 1 : 0
			);
	}
	SaveMap();
}
static void MarumeDot(int x, int y, int w, int h)
{
	int ctr[2] = { 0, 0 };
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
			ctr[getTableCell(Map, sx, sy)]++;
		}
	}
	if (ctr[0] != ctr[1])
	{
		setTableCell(Map, x, y, ctr[0] < ctr[1] ? 1 : 0);
	}
}
static void MarumeMap(int count)
{
	int x, y, w, h;

	LoadMap();

	w = getTableWidth(Map);
	h = getTableHeight(Map);

	while (0 < count)
	{
		x = mt19937_rnd(w);
		y = mt19937_rnd(h);

		MarumeDot(x, y, w, h);

		count--;
	}
	SaveMap();
}
static void WipeMarumeMap(int count, int mode) // mode: 0 - 7
{
	int x, y, w, h;

	LoadMap();

	w = getTableWidth(Map);
	h = getTableHeight(Map);

	while (0 < count)
	{
		for (x = 0; x < w; x++)
		for (y = 0; y < h; y++)
		{
			int xc = x;
			int yc = y;

			if (mode & 1) xc = w - 1 - xc;
			if (mode & 2) yc = h - 1 - yc;
			if (mode & 4)
			{
				int tmp = xc;
				xc = yc;
				yc = tmp;
			}
			MarumeDot(xc, yc, w, h);
		}
		count--;
	}
	SaveMap();
}
static void FrameMap(
	int atsusa,
	int ns, int nc, // north_start, north_count
	int ss, int sc, // south_start, south_count
	int ws, int wc, // west_start, west_count
	int es, int ec // east_start, east_count
	)
{
	int c, i, w, h;

	LoadMap();

	w = getTableWidth(Map);
	h = getTableHeight(Map);

	errorCase(w < 1);
	errorCase(h < 1);

	for (c = 0; c < atsusa; c++)
	{
		for (i = 0; i < w; i++)
		{
			setTableCell(Map, i, c, 1);
			setTableCell(Map, i, h - 1 - c, 1);
		}
		for (i = 0; i < h; i++)
		{
			setTableCell(Map, c, i, 1);
			setTableCell(Map, w - 1 - c, i, 1);
		}
	}
	for (c = 0; c < atsusa; c++)
	{
		for (i = 0; i < nc; i++) setTableCell(Map, ns + i, c, 0);
		for (i = 0; i < sc; i++) setTableCell(Map, ss + i, h - 1 - c, 0);
		for (i = 0; i < wc; i++) setTableCell(Map, c, ws + i, 0);
		for (i = 0; i < ec; i++) setTableCell(Map, w - 1 - c, es + i, 0);
	}
	SaveMap();
}
static void BoxMap(int x, int y, int boxW, int boxH, int flag)
{
	int w, h;
	int xc;
	int yc;

	LoadMap();

	w = getTableWidth(Map);
	h = getTableHeight(Map);

	flag = flag ? 1 : 0;

	for (xc = 0; xc < boxW; xc++)
	for (yc = 0; yc < boxH; yc++)
	{
		int sx = x + xc;
		int sy = y + yc;

		if (
			0 <= sx && sx < w &&
			0 <= sy && sy < h
			)
		{
			setTableCell(Map, sx, sy, flag);
		}
	}
	SaveMap();
}
static void FM_Put2x2Tree(autoTable_t *newMap, int origX, int origY)
{
	int x, y, w, h;

	w = getTableWidth(Map);
	h = getTableHeight(Map);

	for (x = origX; x < w - 1; x += 2)
	for (y = origY; y < h - 1; y += 2)
	{
		int flag = 0;
		int xc;
		int yc;

		for (xc = 0; xc < 2; xc++)
		for (yc = 0; yc < 2; yc++)
		{
			if (getTableCell(Map, x + xc, y + yc)) flag = 1;
		}
		for (xc = 0; xc < 2; xc++)
		for (yc = 0; yc < 2; yc++)
		{
			setTableCell(
				newMap,
				x + xc,
				y + yc,
				getTableCell(newMap, x + xc, y + yc) | flag
				);
		}
	}
}
static void ForestMap(void)
{
	autoTable_t *newMap = newTable(getZero, noop_u);
	int i, w, h;

	LoadMap();

	w = getTableWidth(Map);
	h = getTableHeight(Map);

	errorCase(w < 1);
	errorCase(h < 1);

	resizeTable(newMap, w, h);

	FM_Put2x2Tree(newMap, 0, 0);
	FM_Put2x2Tree(newMap, 1, 1);

	releaseTable(Map);
	Map = newMap;

	SaveMap();
}

int main(int argc, char **argv)
{
	Map = newTable(getZero, noop_u);

	if (argIs("/S"))
		mt19937_initRnd(toValue(nextArg()));
	else
		mt19937_initRnd(1);

readArgs:
	if (argIs("/I"))
	{
		InitMap(
			toValue(getArg(0)),
			toValue(getArg(1)),
			toValue(getArg(2))
			);
		skipArg(3);
		goto readArgs;
	}
	if (argIs("/E"))
	{
		ExpandMap(toValue(nextArg()));
		goto readArgs;
	}
	if (argIs("/R"))
	{
		RandomizeMap(
			toValue(getArg(0)),
			toValue(getArg(1))
			);
		skipArg(2);
		goto readArgs;
	}
	if (argIs("/M"))
	{
		MarumeMap(toValue(nextArg()));
		goto readArgs;
	}
	if (argIs("/W"))
	{
		WipeMarumeMap(
			toValue(getArg(0)),
			toValue(getArg(1))
			);
		skipArg(2);
		goto readArgs;
	}
	if (argIs("/F"))
	{
		FrameMap(
			toValue(getArg(0)),
			toValue(getArg(1)),
			toValue(getArg(2)),
			toValue(getArg(3)),
			toValue(getArg(4)),
			toValue(getArg(5)),
			toValue(getArg(6)),
			toValue(getArg(7)),
			toValue(getArg(8))
			);
		skipArg(9);
		goto readArgs;
	}
	if (argIs("/Box"))
	{
		BoxMap(
			toValue(getArg(0)),
			toValue(getArg(1)),
			toValue(getArg(2)),
			toValue(getArg(3)),
			toValue(getArg(4))
			);
		skipArg(5);
		goto readArgs;
	}
	if (argIs("/Forest"))
	{
		ForestMap();
		goto readArgs;
	}
	if (argIs("/D"))
	{
		execute("START " OUTPUT_BMPFILE);
		goto readArgs;
	}
	if (argIs("/N"))
	{
		LoadMap();
		SaveMap();
		goto readArgs;
	}
	errorCase(hasArgs(1));
}
