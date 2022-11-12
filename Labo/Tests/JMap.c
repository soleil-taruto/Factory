#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\bmptbl.h"

static autoTable_t *Bmp;

static void MakeBmp(void)
{
	int c1;
	int c2;

	Bmp = newTable(getZero, noop_u);

	resizeTable(Bmp, 256, 256);

	for (c1 = 0; c1 < 256; c1++)
	for (c2 = 0; c2 < 256; c2++)
	{
		int chr = c1 * 256 + c2;
		uint color;

		if (isJChar(chr))
			color = 0x0000ff;
		else
			color = 0xffffff;

		setTableCell(Bmp, c2, c1, color);
	}
}
int main(int argc, char **argv)
{
	MakeBmp();
	tWriteBMPFile(c_getOutFile("JMap.bmp"), Bmp);
	openOutDir();
}
