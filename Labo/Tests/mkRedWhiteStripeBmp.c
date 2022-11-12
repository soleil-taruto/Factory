#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\bmptbl.h"

#define XSIZE 1700
#define YSIZE 1700

static autoTable_t *Image;

static void MakeImage(void)
{
	int x;
	int y;

	Image = newTable(getZero, noop_u);

	resizeTable(Image, XSIZE, YSIZE);

	for (x = 0; x < XSIZE; x++)
	for (y = 0; y < YSIZE; y++)
	{
		int color = 0;

		if (0 < x && x < XSIZE - 1 && 0 < y && y < YSIZE - 1)
		{
			color = (x + y) % 5 < 3 ? 0xff0000 : 0xffffff;
		}
		setTableCell(Image, x, y, color);
	}
}
int main(int argc, char **argv)
{
	MakeImage();
	tWriteBMPFile(c_getOutFile("RedWhiteStripe.bmp"), Image);
	openOutDir();
}
