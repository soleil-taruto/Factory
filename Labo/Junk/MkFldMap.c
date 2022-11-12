#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\autoTable.h"
#include "C:\Factory\Common\Options\CRRandom.h"
#include "C:\Factory\SubTools\libs\bmpTbl.h"

#define CELL_BLACK 0x004000
#define CELL_WHITE 0xffffa0

static void MkFldMap(uint mapW, uint mapH, uint noChgMax)
{
	autoTable_t *map = newTable(getZero, noop_u);
	uint noChgCnt = 0;

	LOGPOS();

	resizeTable(map, mapW, mapH);

	{
		uint x;
		uint y;

		for (x = 0; x < mapW; x++)
		for (y = 0; y < mapH; y++)
		{
			setTableCell(map, x, y, mt19937_rnd(2) ? CELL_BLACK : CELL_WHITE);
		}
	}

	while (noChgCnt < noChgMax)
	{
		int x = mt19937_rnd(mapW);
		int y = mt19937_rnd(mapH);
		int sx;
		int sy;
		uint black = 0;
		uint cell;

		for (sx = -1; sx <= 1; sx++)
		for (sy = -1; sy <= 1; sy++)
		{
			int xx = x + sx;
			int yy = y + sy;

			xx += mapW;
			yy += mapH;

			xx %= mapW;
			yy %= mapH;

			if (getTableCell(map, xx, yy) == CELL_BLACK)
			{
				black++;
			}
		}
		cell = black < 5 ? CELL_WHITE : CELL_BLACK;

		if (getTableCell(map, x, y) != cell)
			setTableCell(map, x, y, cell);
		else
			noChgCnt++;
	}
	tWriteBMPFile(getOutFile_x(xcout("%u_%u_%u.bmp", mapW, mapH, noChgMax)), map);

	releaseTable(map);

	LOGPOS();
}
static void Go3(uint wh, uint noChgMax)
{
	MkFldMap(wh, wh, noChgMax);
}
static void Go2(uint wh)
{
	Go3(wh, 3000);
	Go3(wh, 100000);
	Go3(wh, 3000000);
}
static void Go(void)
{
	Go2(10);
	Go2(30);
	Go2(100);
	Go2(300);
	Go2(1000);
	Go2(3000);
}
int main(int argc, char **argv)
{
	mt19937_initCRnd();
	Go();
	openOutDir();
}
