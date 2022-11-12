#include "C:\Factory\Common\all.h"
#include "libs\bmptbl.h"

static int MixMode = 'A'; // "ABSX";

static void MixBmp(char *rFile1, char *rFile2, char *wFile)
{
	autoTable_t *bmp1 = tReadBMPFile(rFile1);
	autoTable_t *bmp2 = tReadBMPFile(rFile2);
	uint w1, w2, w;
	uint h1, h2, h;
	autoTable_t *wBmp;
	uint x;
	uint y;

	w1 = getTableWidth(bmp1);
	h1 = getTableHeight(bmp1);
	w2 = getTableWidth(bmp2);
	h2 = getTableHeight(bmp2);

	if (w1 != w2 || h1 != h2)
	{
		cout("サイズが違います。\n");
		cout("1: %u %u\n", w1, h1);
		cout("2: %u %u\n", w2, h2);
	}
	w = m_max(w1, w2);
	h = m_max(h1, h2);

	wBmp = newTable(getZero, noop_u);

	resizeTable(bmp1, w, h);
	resizeTable(bmp2, w, h);
	resizeTable(wBmp, w, h);

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		uint c1 = getTableCell(bmp1, x, y);
		uint c2 = getTableCell(bmp2, x, y);
		uint c3;
		uint cR1;
		uint cG1;
		uint cB1;
		uint cR2;
		uint cG2;
		uint cB2;
		uint cR3;
		uint cG3;
		uint cB3;

		cR1 = c1 >> 16;
		cG1 = c1 >> 8 & 0xff;
		cB1 = c1 & 0xff;
		cR2 = c2 >> 16;
		cG2 = c2 >> 8 & 0xff;
		cB2 = c2 & 0xff;

		switch (MixMode)
		{
		case 'A':
			cR3 = (cR1 + cR2) / 2;
			cG3 = (cG1 + cG2) / 2;
			cB3 = (cB1 + cB2) / 2;
			break;

		case 'B':
			cR3 = cR1 == cR2 ? 0 : 255;
			cG3 = cG1 == cG2 ? 0 : 255;
			cB3 = cB1 == cB2 ? 0 : 255;
			break;

		case 'S':
			cR3 = cR1 + 0x100 - cR2 & 0xff;
			cG3 = cG1 + 0x100 - cG2 & 0xff;
			cB3 = cB1 + 0x100 - cB2 & 0xff;
			break;

		case 'X':
			cR3 = cR1 ^ cR2;
			cG3 = cG1 ^ cG2;
			cB3 = cB1 ^ cB2;
			break;

		default:
			error();
		}
		c3 = cR3 << 16 | cG3 << 8 | cB3;
		setTableCell(wBmp, x, y, c3);
	}
	releaseTable(bmp1);
	releaseTable(bmp2);
	tWriteBMPFile_cx(wFile, wBmp);
}
int main(int argc, char **argv)
{
	if (argIs("/B"))
	{
		MixMode = 'B';
	}
	if (argIs("/S"))
	{
		MixMode = 'S';
	}
	if (argIs("/X"))
	{
		MixMode = 'X';
	}

	{
		char *rFile1;
		char *rFile2;
		char *wFile;

		rFile1 = nextArg();
		rFile2 = nextArg();
		wFile = nextArg();

		MixBmp(rFile1, rFile2, wFile);
	}
}
