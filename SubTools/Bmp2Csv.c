#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csv.h"
#include "libs\bmp.h"

static int ThreeCellMode;

static void Bmp2Csv(char *rFile, char *wFile)
{
	autoList_t *bmp = readBMPFile(rFile);
	autoList_t *csv = newList();
	autoList_t *rRow;
	autoList_t *wRow;
	uint rowidx;
	uint colidx;
	uint dot;

	foreach (bmp, rRow, rowidx)
	{
		wRow = newList();

		foreach (rRow, dot, colidx)
		{
			if (ThreeCellMode)
			{
				addElement(wRow, (uint)xcout("%u", dot >> 8 * 2 & 0xff));
				addElement(wRow, (uint)xcout("%u", dot >> 8 * 1 & 0xff));
				addElement(wRow, (uint)xcout("%u", dot >> 8 * 0 & 0xff));
			}
			else
			{
				addElement(wRow, (uint)xcout("%06x", dot));
			}
		}
		addElement(csv, (uint)wRow);
	}
	writeCSVFile(wFile, csv);

	releaseDim_BR(bmp, 2, NULL);
	releaseDim(csv, 2);
}
static void Csv2Bmp(char *rFile, char *wFile)
{
	autoList_t *csv = readCSVFileTR(rFile);
	autoList_t *bmp = newList();
	autoList_t *rRow;
	autoList_t *wRow;
	uint rowidx;
	uint colidx;
	char *cell;

	foreach (csv, rRow, rowidx)
	{
		wRow = newList();

		if (ThreeCellMode)
		{
			for (colidx = 0; colidx < getCount(rRow) / 3; colidx++)
			{
				addElement(
					wRow,
					toValue(getLine(rRow, colidx * 3 + 0)) << 8 * 2 |
					toValue(getLine(rRow, colidx * 3 + 1)) << 8 * 1 |
					toValue(getLine(rRow, colidx * 3 + 2)) << 8 * 0
					);
			}
		}
		else
		{
			foreach (rRow, cell, colidx)
			{
				addElement(wRow, toValueDigits(cell, hexadecimal));
			}
		}
		addElement(bmp, (uint)wRow);
	}
	writeBMPFile(wFile, bmp);

	releaseDim(csv, 2);
	releaseDim_BR(bmp, 2, NULL);
}
int main(int argc, char **argv)
{
	if (argIs("/3"))
	{
		ThreeCellMode = 1;
	}

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
	if (argIs("/B2B"))
	{
		char *midFile = makeTempPath(NULL);

		Bmp2Csv(getArg(0), midFile);
		Csv2Bmp(midFile, getArg(1));

		removeFile(midFile);
		memFree(midFile);
		return;
	}
}
