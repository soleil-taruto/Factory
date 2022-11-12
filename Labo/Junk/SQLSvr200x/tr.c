#include "C:\Factory\Common\all.h"
#include "libs\TableReader.h"
#include "libs\csv.h"

static void AddLine(autoList_t **p_lines, char *line)
{
	if (!*p_lines)
		*p_lines = newList();

	addElement(*p_lines, (uint)strx(line));
}
static autoList_t *WhereColNames;
static autoList_t *WhereColValues;
static autoList_t *ResultColNames;

static void ReadColArgs(void)
{
readArgs:
	if (argIs("+"))
	{
		AddLine(&WhereColNames, nextArg());
		goto readArgs;
	}
	if (argIs("="))
	{
		AddLine(&WhereColValues, nextArg());
		goto readArgs;
	}
	if (argIs(":"))
	{
		AddLine(&ResultColNames, nextArg());
		goto readArgs;
	}

	errorCase(hasArgs(1)); // 不明なコマンド引数

	if (!WhereColNames && !WhereColValues)
	{
		AddLine(&WhereColNames, "$");
		AddLine(&WhereColValues, "");
	}
}
int main(int argc, char **argv)
{
	if (argIs("/S"))
	{
		autoList_t *colnms = TR_GetSchema(nextArg());
		char *colnm;
		uint colidx;

		foreach (colnms, colnm, colidx)
			cout("%s\n", colnm);

		releaseDim(colnms, 1);
		return;
	}
	if (argIs("/T"))
	{
		char *csvFile = nextArg();
		autoList_t *values;
		char *value;
		uint index;

		ReadColArgs();
		values = TR_SelectTop1WhereAnd_cxxx(csvFile, WhereColNames, WhereColValues, ResultColNames);

		foreach (values, value, index)
			cout("%s\n", value);

		releaseDim(values, 1);
		return;
	}
	if (argIs("/R"))
	{
		char *csvFile;
		char *destFile;
		char *destCsvFile;
		uint64 rowcnt;
		uint64 rowidx;
		uint colidx;
		FILE *rfp;
		FILE *wfp;

		/*
			csvFile == destCsvFile -> OK
		*/
		csvFile = nextArg();
		destFile = makeTempPath("mid");
		destCsvFile = nextArg();

		ReadColArgs();
		errorCase(!ResultColNames);
		rowcnt = TR_SelectWhereAnd_cxxxc(csvFile, WhereColNames, WhereColValues, copyLines(ResultColNames), destFile);

		rfp = fileOpen(destFile, "rt");
		wfp = fileOpen(destCsvFile, "wt");

		RowToCSVStream(wfp, ResultColNames);

		for (rowidx = 0; rowidx < rowcnt; rowidx++)
		{
			autoList_t *row = newList();

			for (colidx = 0; colidx < getCount(ResultColNames); colidx++)
			{
				addElement(row, (uint)neReadLine(rfp));
			}
			RowToCSVStream_x(wfp, row);
		}
		errorCase(readLine(rfp)); // ? 余計な行がある。

		fileClose(rfp);
		removeFile(destFile);
		fileClose(wfp);

		releaseDim(ResultColNames, 1);
		memFree(destFile);
		return;
	}
	if (argIs("/D"))
	{
		error(); // HACK: DELETE
		return;
	}
	if (argIs("/I"))
	{
		error(); // HACK: INSERT
		return;
	}
	if (argIs("/C"))
	{
		error(); // HACK: CREATE
		return;
	}

	error(); // 不明なコマンド引数
}
