#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\autoTable.h"
#include "C:\Factory\Common\Options\Random.h"
#include "C:\Factory\SubTools\libs\bmp.h"
#include "C:\Factory\Common\Options\Sequence.h"

#define OUTPUT_BMPFILE "Map.bmp"

static void PaintMap(autoTable_t *map, uint x, uint y, uint target, uint dest)
{
	autoList_t *xList = newList();
	autoList_t *yList = newList();
	uint rIndex = 0;

	addElement(xList, x);
	addElement(yList, y);

	while (rIndex < getCount(xList))
	{
		x = getElement(xList, rIndex);
		y = getElement(yList, rIndex);
		rIndex++;

		if (
			x < getTableWidth(map) &&
			y < getTableHeight(map) &&
			getTableCell(map, x, y) == target
			)
		{
			setTableCell(map, x, y, dest);

			addElement(xList, x - 1);
			addElement(xList, x + 1);
			addElement(xList, x);
			addElement(xList, x);

			addElement(yList, y);
			addElement(yList, y);
			addElement(yList, y - 1);
			addElement(yList, y + 1);
		}
	}
	releaseAutoList(xList);
	releaseAutoList(yList);
}
static uint MapGetCount(autoTable_t *map, uint target)
{
	uint count = 0;
	uint x;
	uint y;

	for (x = 0; x < getTableWidth(map); x++)
	for (y = 0; y < getTableHeight(map); y++)
	{
		if (getTableCell(map, x, y) == target)
		{
			count++;
		}
	}
	return count;
}

static autoTable_t *Map;
static uint Map_W;
static uint Map_H;
static uint RndWlRtNumer;
static uint RndWlRtDenom;

static autoList_t *Bmp;

#define COLOR_WALL 0x000040
#define COLOR_GROUND 0x0000ff
#define COLOR_GROUND_MAXIM 0x8080ff
#define COLOR_GROUND_MAXIM2ND 0x4040ff
#define COLOR_GROUND_NEAREST 0xffff80

// MM_ >

static void MM_Randomize(void)
{
	uint x;
	uint y;

	for (x = 0; x < Map_W; x++)
	for (y = 0; y < Map_H; y++)
	{
		setTableCell(Map, x, y, mt19937_rnd(RndWlRtDenom) < RndWlRtNumer ? COLOR_WALL : COLOR_GROUND);
	}
}
static void MM_BattlePos(int x, int y)
{
	int xc;
	int yc;
	uint wallcnt = 0;

	for (xc = -1; xc <= 1; xc++)
	for (yc = -1; yc <= 1; yc++)
	{
		int sx = x + xc;
		int sy = y + yc;

		// è„â∫ÉãÅ[Évâª
		{
			sx += Map_W;
			sx %= Map_W;
			sy += Map_H;
			sy %= Map_H;
		}

		if (
			0 <= sx && sx < Map_W &&
			0 <= sy && sy < Map_H
			)
		{
			if (getTableCell(Map, sx, sy) == COLOR_WALL)
			{
				wallcnt++;
			}
		}
		else
		{
			wallcnt++;
		}
		setTableCell(Map, x, y, 5 <= wallcnt ? COLOR_WALL : COLOR_GROUND);
	}
}
static void MM_Battle(void)
{
	autoList_t *order = createSq(Map_W * Map_H, 0, 1);
	uint index;
	uint pos;

	shuffle(order);

	foreach (order, pos, index)
	{
		uint x = pos / Map_H;
		uint y = pos % Map_H;

		MM_BattlePos(x, y);
	}
	releaseAutoList(order);
}
static void MM_Paint(uint x, uint y, uint target, uint dest)
{
	PaintMap(Map, x, y, target, dest);
}
static void MM_AroundPos(uint x, uint y)
{
	if (getTableCell(Map, x, y) == COLOR_GROUND)
	{
		MM_Paint(x, y, COLOR_GROUND, COLOR_WALL);
	}
}
static void MM_Around(void)
{
	uint x;
	uint y;

	for (x = 0; x < Map_W; x++)
	{
		MM_AroundPos(x, 0);
		MM_AroundPos(x, Map_H - 1);
	}
	for (y = 0; y < Map_H; y++)
	{
		MM_AroundPos(0, y);
		MM_AroundPos(Map_W - 1, y);
	}
}

static uint Maxim_X;
static uint Maxim_Y;
static uint MaximSize;

static uint Maxim2nd_X;
static uint Maxim2nd_Y;
static uint Maxim2ndSize;

static void MM_FindMaximum(void)
{
	autoTable_t *szMap = newTable(getZero, noop_u);
	uint x;
	uint y;
	uint nuttaCell = 2;

	resizeTable(szMap, Map_W, Map_H);

	for (x = 0; x < Map_W; x++)
	for (y = 0; y < Map_H; y++)
	{
		setTableCell(szMap, x, y, getTableCell(Map, x, y) == COLOR_GROUND ? 1 : 0);
	}

	Maxim_X = 0;
	Maxim_Y = 0;
	MaximSize = 0;

	Maxim2nd_X = 0;
	Maxim2nd_Y = 0;
	Maxim2ndSize = 0;

	for (x = 0; x < Map_W; x++)
	for (y = 0; y < Map_H; y++)
	{
		if (getTableCell(szMap, x, y) == 1)
		{
			uint count;

			cmdTitle_x(xcout("MakeMap - %u %u", x, y));

			PaintMap(szMap, x, y, 1, nuttaCell);
			count = MapGetCount(szMap, nuttaCell);

			if (MaximSize < count)
			{
				cout("M1: %u %u %u\n", x, y, count);

				Maxim2nd_X = Maxim_X;
				Maxim2nd_Y = Maxim_Y;
				Maxim2ndSize = MaximSize;

				Maxim_X = x;
				Maxim_Y = y;
				MaximSize = count;
			}
			else if (Maxim2ndSize < count)
			{
				cout("M2: %u %u %u\n", x, y, count);

				Maxim2nd_X = x;
				Maxim2nd_Y = y;
				Maxim2ndSize = count;
			}
			nuttaCell++;
		}
	}
	releaseTable(szMap);

	MM_Paint(Maxim_X, Maxim_Y, COLOR_GROUND, COLOR_GROUND_MAXIM);
	MM_Paint(Maxim2nd_X, Maxim2nd_Y, COLOR_GROUND, COLOR_GROUND_MAXIM2ND);

	cout("ìáÇÃêî: %u\n", nuttaCell - 1);
}

static uint NF_X;
static uint NF_Y;
static uint64 NF_Sqr;

static void MM_FindNearestFrom(int fx, int fy, uint target)
{
	int x;
	int y;

	NF_Sqr = UINT64MAX;

	for (x = 0; x < Map_W; x++)
	for (y = 0; y < Map_H; y++)
	{
		if (getTableCell(Map, x, y) == target)
		{
			uint dx = abs(x - fx);
			uint dy = abs(y - fy);
			uint64 sqr;

			sqr = (uint64)dx * dx + (uint64)dy * dy;

			if (sqr < NF_Sqr)
			{
				NF_X = x;
				NF_Y = y;
				NF_Sqr = sqr;
			}
		}
	}
	errorCase(NF_Sqr == UINT64MAX); // ? not found
}

static uint Nrst1_X;
static uint Nrst1_Y;
static uint Nrst2_X;
static uint Nrst2_Y;

static void MM_FindNearest(void)
{
	uint c;

	Nrst1_X = Maxim_X;
	Nrst1_Y = Maxim_Y;

	for (c = 10; c; c--)
//	for (c = 2; c; c--)
	{
		MM_FindNearestFrom(Nrst1_X, Nrst1_Y, COLOR_GROUND_MAXIM2ND);
		Nrst2_X = NF_X;
		Nrst2_Y = NF_Y;

		MM_FindNearestFrom(Nrst2_X, Nrst2_Y, COLOR_GROUND_MAXIM);
		Nrst1_X = NF_X;
		Nrst1_Y = NF_Y;
	}

	setTableCell(Map, Nrst1_X, Nrst1_Y, COLOR_GROUND_NEAREST);
	setTableCell(Map, Nrst2_X, Nrst2_Y, COLOR_GROUND_NEAREST);
}
static void MM_JoinNearest(void)
{
	int x1 = Nrst1_X;
	int y1 = Nrst1_Y;
	int x2 = Nrst2_X;
	int y2 = Nrst2_Y;

#if 1
	if (x2 < x1) m_swap(x1, x2, int);
	if (y2 < y1) m_swap(y1, y2, int);

	{
		int x;
		int y;

		for (x = x1; x <= x2; x++)
		for (y = y1; y <= y2; y++)
		{
			if (x == x1 || x == x2 || y == y1 || y == y2)
			{
				setTableCell(Map, x, y, COLOR_GROUND_NEAREST);
			}
		}
	}
#else // OLD
	while (x1 != x2 || y1 != y2)
	{
		if (abs(x1 - x2) < abs(y1 - y2))
		{
			y1 += m_sign(y2 - y1);
			y2 += m_sign(y1 - y2);
		}
		else
		{
			x1 += m_sign(x2 - x1);
			x2 += m_sign(x1 - x2);
		}

		setTableCell(Map, x1, y1, COLOR_GROUND_NEAREST);
		setTableCell(Map, x2, y2, COLOR_GROUND_NEAREST);
	}
#endif
}
static void MM_ResetGround(void)
{
	uint x;
	uint y;

	for (x = 0; x < Map_W; x++)
	for (y = 0; y < Map_H; y++)
	{
		switch (getTableCell(Map, x, y))
		{
		case COLOR_GROUND_MAXIM:
		case COLOR_GROUND_MAXIM2ND:
		case COLOR_GROUND_NEAREST:
			setTableCell(Map, x, y, COLOR_GROUND);
			break;
		}
	}
}

// < MM_

static void MakeBmp(void)
{
	int x;
	int y;

	Bmp = makeTable(Map_W, Map_H, 0);

	for (x = 0; x < Map_W; x++)
	for (y = 0; y < Map_H; y++)
	{
		*getTablePoint(Bmp, x, y) = getTableCell(Map, x, y);
	}
}
int main(int argc, char **argv)
{
	uint rndSeed;
	uint c;

	Map = newTable(getZero, noop_u);
	Map_W = toValue(nextArg());
	Map_H = toValue(nextArg());
	rndSeed = toValue(nextArg());
	RndWlRtNumer = toValue(nextArg());
	RndWlRtDenom = toValue(nextArg());

	errorCase(Map_W < 1 || 0xffff < Map_W);
	errorCase(Map_H < 1 || 0xffff < Map_H);
	errorCase(RndWlRtNumer < 1 || 10000 < Map_H);
	errorCase(RndWlRtDenom < 1 || 10000 < Map_H);
	errorCase(RndWlRtDenom < RndWlRtNumer);

	mt19937_initRnd(rndSeed);
	resizeTable(Map, Map_W, Map_H);

	// MM_ >

	LOGPOS();
	MM_Randomize();
	LOGPOS();
	MM_Battle();
	LOGPOS();
	MM_Around();
	LOGPOS();

	for (c = 10; c; c--)
	{
		cout("[%u]\n", c);

		LOGPOS();
		MM_FindMaximum();
		LOGPOS();

		if (!Maxim2ndSize) // ? 2Ç¬å©Ç¬Ç©ÇÁÇ»Ç©Ç¡ÇΩÅB
			break;

		LOGPOS();
		MM_FindNearest();
		LOGPOS();
		MM_JoinNearest();
		LOGPOS();
		MM_ResetGround();
		LOGPOS();
	}

	LOGPOS();
	MM_ResetGround();
	LOGPOS();

	// < MM_

	MakeBmp();
	writeBMPFile(c_getOutFile(OUTPUT_BMPFILE), Bmp);
	execute_x(xcout("START mspaint %s", c_getOutFile(OUTPUT_BMPFILE)));
}
