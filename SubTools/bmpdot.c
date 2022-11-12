#include "C:\Factory\Common\all.h"
#include "libs\bmptbl.h"

/*
	mode: "XN" (max, min)
	color: "RGB"
*/
static void CheckColorMinMax(char *file, int mode, int rgb)
{
	autoTable_t *bmp = tReadBMPFile(file);
	int maxMode;
	uint x;
	uint y;
	uint retcol;

	cout("chkColMinMax\n");
	cout("mode: %c\n", mode);
	cout("rgb: %c\n", rgb);

	switch (mode)
	{
	case 'X': retcol = 0; break;
	case 'N': retcol = 255; break;

	default:
		error();
	}

	for (x = 0; x < getTableWidth(bmp); x++)
	for (y = 0; y < getTableHeight(bmp); y++)
	{
		uint color = getTableCell(bmp, x, y);

		switch (rgb)
		{
		case 'R': color = color >> 16 & 0xff; break;
		case 'G': color = color >>  8 & 0xff; break;
		case 'B': color = color >>  0 & 0xff; break;

		default:
			error();
		}
		switch (mode)
		{
		case 'X': m_maxim(retcol, color); break;
		case 'N': m_minim(retcol, color); break;

		defualt:
			error();
		}
	}
	cout("retcol: %u\n", retcol);

	termination(retcol);
}
int main(int argc, char **argv)
{
	if (argIs("/XR"))
	{
		CheckColorMinMax(nextArg(), 'X', 'R');
		return; // dummy
	}
}
