/*
	CsvToJson.exe [入力CSVファイル 出力JSONファイル]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csv.h"

static void CsvToJson(char *rFile, char *wFile)
{
	autoList_t *csv = readCSVFileTR(rFile);
	autoList_t *head;
	uint rowidx;
	FILE *wfp;

	head = getList(csv, 0);

	// HACK: 出力フォーマット適当..

	wfp = fileOpen(wFile, "wt");
	writeLine(wfp, "[");

	for (rowidx = 1; rowidx < getCount(csv); rowidx++)
	{
		autoList_t *row = getList(csv, rowidx);
		char *cell;
		uint colidx;

		writeLine(wfp, "\t{");

		foreach (row, cell, colidx)
		{
			writeLine_x(wfp, xcout("\t\t\"%s\": \"%s\",", getLine(head, colidx), getLine(row, colidx)));
		}
		writeLine(wfp, "\t},");
	}
	writeLine(wfp, "]");
	fileClose(wfp);
}
int main(int argc, char **argv)
{
	if (hasArgs(2))
	{
		CsvToJson(getArg(0), getArg(1));
		return;
	}

	CsvToJson(c_dropFile(), c_getOutFile("output.json"));
	openOutDir();
}
