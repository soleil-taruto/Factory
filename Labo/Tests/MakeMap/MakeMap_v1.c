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
static void Nearize(int *var_p, int target)
{
	*var_p += m_sign(target - *var_p);
}

static autoTable_t *Map;
static uint Map_W;
static uint Map_H;
static uint RndWlRtNumer;
static uint RndWlRtDenom;

static autoList_t *Bmp;

#define COLOR_WALL 0x000040
#define COLOR_GROUND 0x0000ff

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

		// ã‰ºƒ‹[ƒv‰»
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

static autoTable_t *IslandMap;
static autoList_t *IslandXPosList;
static autoList_t *IslandYPosList;
static uint IslandCount;

static void MM_Island(void)
{
	uint x;
	uint y;

	if (!IslandMap)
	{
		IslandMap = newTable(getZero, noop_u);
		IslandXPosList = newList();
		IslandYPosList = newList();
	}
	resizeTable(IslandMap, Map_W, Map_H);
	resetTable(IslandMap);
	setCount(IslandXPosList, 0);
	setCount(IslandYPosList, 0);
	IslandCount= 0;

	for (x = 0; x < Map_W; x++)
	for (y = 0; y < Map_H; y++)
	{
		setTableCell(IslandMap, x, y, getTableCell(Map, x, y) == COLOR_GROUND ? UINTMAX : 0);
	}
	for (x = 0; x < Map_W; x++)
	for (y = 0; y < Map_H; y++)
	{
		if (getTableCell(IslandMap, x, y) == UINTMAX)
		{
			PaintMap(IslandMap, x, y, UINTMAX, IslandCount + 1);
			addElement(IslandXPosList, x);
			addElement(IslandYPosList, y);
			IslandCount++;
		}
	}

	cout("“‡‚Ì”: %u\n", IslandCount);
}

typedef struct Pos_st
{
	uint X;
	uint Y;
}
Pos_t;

static Pos_t MakePos(uint x, uint y)
{
	Pos_t pos;

	pos.X = x;
	pos.Y = y;

	return pos;
}

typedef struct PairPos_st
{
	Pos_t From;
	Pos_t Dest;
}
PairPos_t;

static PairPos_t *CreatePairPos(void)
{
	return nb_(PairPos_t);
}
static void ReleasePairPos(PairPos_t *i)
{
	memFree(i);
}
static autoTable_t *NearestTable; // (from, dest)

static PairPos_t *GetNearestCell(uint x, uint y)
{
	return (PairPos_t *)getTableCell(NearestTable, x, y);
}
static Pos_t MM_GetNearest(Pos_t *fromPos, uint target)
{
	uint nrstX = 0;
	uint nrstY = 0;
	uint64 nrstSqr = UINT64MAX;
	uint x;
	uint y;

	for (x = 0; x < Map_W; x++)
	for (y = 0; y < Map_H; y++)
	{
		if (getTableCell(IslandMap, x, y) == target)
		{
			uint dx = abs((int)fromPos->X - (int)x);
			uint dy = abs((int)fromPos->Y - (int)y);
			uint64 sqr;

			sqr = (uint64)dx * dx + (uint64)dy * dy;

			if (sqr < nrstSqr)
			{
				nrstX = x;
				nrstY = y;
				nrstSqr = sqr;
			}
		}
	}
	errorCase(nrstSqr == UINT64MAX); // ? not found

	return MakePos(nrstX, nrstY);
}
static void MM_NearestFromDest(PairPos_t *i, uint fromIsland, uint destIsland)
{
	uint c;

	i->From.X = getElement(IslandXPosList, fromIsland);
	i->From.Y = getElement(IslandYPosList, fromIsland);

	for (c = 10; c; c--)
	{
		Pos_t fromPos;

		i->Dest = MM_GetNearest(&i->From, destIsland + 1);
		fromPos = MM_GetNearest(&i->Dest, fromIsland + 1);

		if (fromPos.X == i->From.X && fromPos.Y == i->From.Y)
			break;

		i->From = fromPos;
	}
}
static void MM_Nearest(void)
{
	uint fromIsland;
	uint destIsland;

LOGPOS();
	if (!NearestTable)
		NearestTable = newTable((uint (*)(void))CreatePairPos, (void (*)(uint))ReleasePairPos);

	resizeTable(NearestTable, IslandCount, IslandCount);
	resetTable(NearestTable);
LOGPOS();

	for (fromIsland = 0; fromIsland < IslandCount; fromIsland++)
	for (destIsland = 0; destIsland < IslandCount; destIsland++)
	{
		cmdTitle_x(xcout("MakeMap - %u %u", fromIsland, destIsland));

		if (fromIsland != destIsland)
		{
			MM_NearestFromDest(GetNearestCell(fromIsland, destIsland), fromIsland, destIsland);
		}
	}
}
static void MM_JoinPairPos(PairPos_t i)
{
	while (i.From.X != i.Dest.X || i.From.Y != i.Dest.Y)
	{
		if (abs((int)i.From.X - (int)i.Dest.X) < abs((int)i.From.Y - (int)i.Dest.Y)) // ? Y •ûŒü‚É‰“‚¢
		{
			Nearize(&i.From.Y, i.Dest.Y);
			Nearize(&i.Dest.Y, i.From.Y);
		}
		else // ? X •ûŒü‚É‰“‚¢
		{
			Nearize(&i.From.X, i.Dest.X);
			Nearize(&i.Dest.X, i.From.X);
		}

		setTableCell(Map, i.From.X, i.From.Y, COLOR_GROUND);
		setTableCell(Map, i.Dest.X, i.Dest.Y, COLOR_GROUND);
	}
}
static void MM_JoinNearest(void)
{
	uint fromIsland;
	uint destIsland;

	for (fromIsland = 0; fromIsland < IslandCount; fromIsland++)
	{
		PairPos_t *nearest = NULL;
		uint64 nearestSqr = UINT64MAX;

		for (destIsland = 0; destIsland < IslandCount; destIsland++)
		{
			if (fromIsland != destIsland)
			{
				PairPos_t *i = GetNearestCell(fromIsland, destIsland);
				uint dx;
				uint dy;
				uint64 sqr;

				dx = abs((int)i->From.X - (int)i->Dest.X);
				dy = abs((int)i->From.Y - (int)i->Dest.Y);
				sqr = (uint64)dx * dx + (uint64)dy * dy;

				if (sqr < nearestSqr)
				{
					nearest = i;
					nearestSqr = sqr;
				}
			}
		}
		errorCase(!nearest); // ? not found

		MM_JoinPairPos(*nearest);
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

	for (c = 100; c; c--)
	{
		cout("[%u]\n", c);

		LOGPOS();
		MM_Island();
		LOGPOS();

		if (IslandCount <= 1)
			break;

		LOGPOS();
		MM_Nearest();
		LOGPOS();
		MM_JoinNearest();
		LOGPOS();
	}
	LOGPOS();

	// < MM_

	MakeBmp();
	writeBMPFile(c_getOutFile(OUTPUT_BMPFILE), Bmp);
	execute_x(xcout("START mspaint %s", c_getOutFile(OUTPUT_BMPFILE)));
}
