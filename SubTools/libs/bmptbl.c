#include "bmptbl.h"

autoTable_t *tReadBMPFile(char *file)
{
	autoList_t *table = readBMPFile(file);
	autoTable_t *bmp = newTable(getZero, noop_u);
	uint w;
	uint h;
	uint x;
	uint y;

	w = getCount(getList(table, 0));
	h = getCount(table);

	errorCase(!w || !h); // 2bs

	resizeTable(bmp, w, h);

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		setTableCell(bmp, x, y, getElement(getList(table, y), x));
	}
	releaseDim_BR(table, 2, NULL);
	return bmp;
}
void tWriteBMPFile(char *file, autoTable_t *bmp)
{
	autoList_t *table = newList();
	uint w;
	uint h;
	uint x;
	uint y;

	w = getTableWidth(bmp);
	h = getTableHeight(bmp);

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		putElement(refList(table, y), x, getTableCell(bmp, x, y));
	}
	writeBMPFile_cx(file, table);
}

// _x
void tWriteBMPFile_cx(char *file, autoTable_t *bmp)
{
	tWriteBMPFile(file, bmp);
	releaseTable(bmp);
}
void tWriteBMPFile_xx(char *file, autoTable_t *bmp)
{
	tWriteBMPFile_cx(file, bmp);
	memFree(file);
}
