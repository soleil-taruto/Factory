#include "csvtbl.h"

/*
	ret: NULL‚ÌƒZƒ‹‚ğ‹–‰Â‚·‚é‚±‚Æ‚É’ˆÓ
*/
autoTable_t *tReadCSVFile(char *file)
{
	autoList_t *table = readCSVFileTR(file);
	autoTable_t *csv = newTable(getZero, (void (*)(uint))memFree);
	uint w;
	uint h;
	uint x;
	uint y;

	h = getCount(table);

	if (h)
	{
		w = getCount(getList(table, 0));

		if (w)
		{
			resizeTable(csv, w, h);

			for (x = 0; x < w; x++)
			for (y = 0; y < h; y++)
			{
				setTableCell(csv, x, y, getElement(getList(table, y), x));
			}
			releaseDim_BR(table, 2, NULL);
		}
	}
	return csv;
}
void tWriteCSVFile(char *file, autoTable_t *csv)
{
	autoList_t *table = newList();
	uint w;
	uint h;
	uint x;
	uint y;

	w = getTableWidth(csv);
	h = getTableHeight(csv);

	for (y = 0; y < h; y++)
	{
		autoList_t *row = newList();

		for (x = 0; x < w; x++)
		{
			char *cell = (char *)getTableCell(csv, x, y);

			if (!cell)
				cell = ""; // NULLƒZƒ‹‚Ì‘ã‘Ö’l

			addElement(row, (uint)cell);
		}
		addElement(table, (uint)row);
	}
	writeCSVFile(file, table);
	releaseDim_BR(table, 2, NULL);
}

// _x
void tWriteCSVFile_cx(char *file, autoTable_t *csv)
{
	tWriteCSVFile(file, csv);
	releaseTable(csv);
}
void tWriteCSVFile_xx(char *file, autoTable_t *csv)
{
	tWriteCSVFile_cx(file, csv);
	memFree(file);
}
