#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csvStream.h"

static autoList_t *LoadCsv(char *file)
{
	FILE *fp = fileOpen(file, "rt");
	autoList_t *rows = newList();
	autoList_t *row;

	while (row = readCSVRow(fp))
		addElement(rows, (uint)row);

	fileClose(fp);
	return rows;
}
static void SaveCsv(char *file, autoList_t *rows)
{
	FILE *fp = fileOpen(file, "wt");
	autoList_t *row;
	uint rowidx;

	foreach (rows, row, rowidx)
		writeCSVRow(fp, row);

	fileClose(fp);
}

int main(int argc, char **argv)
{
	autoList_t *rows = LoadCsv(nextArg());
	autoList_t *row;
	uint rowidx;
	char *cell;
	uint colidx;

	foreach (rows, row, rowidx)
	foreach (row, cell, colidx)
		cout("%5uçs%5uóÒñ⁄=%s\n", rowidx, colidx, cell);

	if (hasArgs(1))
		SaveCsv(nextArg(), rows);

	releaseDim(rows, 2);
}
