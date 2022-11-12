#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\bmptbl.h"

static int BokaWidth = 1;

static void Bokashi(char *file)
{
	autoTable_t *bmp = tReadBMPFile(file);
	autoTable_t *nBmp = newTable(getZero, noop_u);
	int x;
	int y;
	int w;
	int h;

	errorCase(BokaWidth < 1);

	w = getTableWidth(bmp);
	h = getTableHeight(bmp);

	resizeTable(nBmp,w, h);

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		int col;
		int colR = 0;
		int colG = 0;
		int colB = 0;
		int dotNum = 0;
		int sx;
		int sy;

		for (sx = -BokaWidth; sx <= BokaWidth; sx++)
		for (sy = -BokaWidth; sy <= BokaWidth; sy++)
		{
			int rx = x + sx;
			int ry = y + sy;

			if (
				m_isRange(rx, 0, w - 1) &&
				m_isRange(ry, 0, h - 1)
				)
			{
				col = getTableCell(bmp, rx, ry);

				colR += col >> 16 & 0xff;
				colG += col >>  8 & 0xff;
				colB += col >>  0 & 0xff;
			}
			dotNum++;
		}
		colR /= dotNum;
		colG /= dotNum;
		colB /= dotNum;

		col = colR << 16 | colG << 8 | colB;

		setTableCell(nBmp, x, y, col);
	}

	tWriteBMPFile(c_getOutFile("Bokashi.bmp"), nBmp);
	openOutDir();

	releaseTable(bmp);
	releaseTable(nBmp);
}
int main(int argc, char **argv)
{
	if (argIs("/W"))
	{
		BokaWidth = atoi(nextArg());
	}

	if (hasArgs(1))
	{
		Bokashi(nextArg());
	}
	else
	{
		for (; ; )
		{
			Bokashi(c_dropFile());
			cout("\n");
		}
	}
}
