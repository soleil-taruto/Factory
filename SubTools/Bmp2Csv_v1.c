#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csv.h"
#include "libs\bmp.h"

static void Bmp2Csv(char *rFile, char *wFile)
{
	autoList_t *table = readBMPFile(rFile);
	autoList_t *row;
	uint rowidx;
	uint cell;
	uint colidx;

	foreach (table, row, rowidx)
	{
		foreach (row, cell, colidx)
		{
			setElement(row, colidx, (uint)xcout("%06x", cell));
		}
	}
	writeCSVFile_cx(wFile, table);
}
static void Csv2Bmp(char *rFile, char *wFile)
{
	autoList_t *table = readCSVFileTR(rFile);
	autoList_t *row;
	uint rowidx;
	char *cell;
	uint colidx;

	foreach (table, row, rowidx)
	{
		foreach (row, cell, colidx)
		{
			setElement(row, colidx, toValueDigits_xc(cell, hexadecimal));
		}
	}
	writeBMPFile_cx(wFile, table);
}
int main(int argc, char **argv)
{
	if (argIs("/B2C"))
	{
		Bmp2Csv(getArg(0), getArg(1));
		return;
	}
	if (argIs("/C2B"))
	{
		Csv2Bmp(getArg(0), getArg(1));
		return;
	}
}
