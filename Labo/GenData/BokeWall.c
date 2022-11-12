#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\bmptbl.h"

#define SCREEN_W 800
#define SCREEN_H 600
#define BOKE_RANGE 64

int main(int argc, char **argv)
{
	autoTable_t *bmp = newTable(getZero, noop_u);
	uint x;
	uint y;

	resizeTable(bmp, SCREEN_W + BOKE_RANGE * 2, SCREEN_H + BOKE_RANGE * 2);

	for (x = 0; x < getTableWidth(bmp); x++)
	for (y = 0; y < getTableHeight(bmp); y++)
	{
		uint farBdr = x + 1;
		uint color;

		m_minim(farBdr, y + 1);
		m_minim(farBdr, getTableWidth(bmp) - x);
		m_minim(farBdr, getTableHeight(bmp) - y);

		color = farBdr * 256 / BOKE_RANGE;
		m_minim(color, 255);
		color = 255 - color;

		setTableCell(bmp, x, y, color * 0x010101);
	}
	tWriteBMPFile_xx(getOutFile("BokeWall.bmp"), bmp);
	openOutDir();
}
