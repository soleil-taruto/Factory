/*
	エラーチェックの方針は TableAccessor.c と同じ。

	[$]カラム
		カラム名のチェックでスルーする。
		引数に条件が必要な関数で無条件を指定したい場合、条件は一つ以上指定しなければならないので、
		[$] = "" という条件を代わりに指定すること。
*/

#include "TableReader.h"

static void CheckColNames(autoList_t *colNames)
{
	char *colName;
	uint colidx;

	errorCase(!colNames);
	errorCase(getCount(colNames) < 1);

	foreach (colNames, colName, colidx)
	{
		errorCase(isEmptyJTkn(colName));
	}
}
static void CheckColValues(autoList_t *colValues, uint colcnt)
{
	char *colValue;
	uint colidx;

	errorCase(!colValues);
	errorCase(getCount(colValues) != colcnt);

	foreach (colValues, colValue, colidx)
	{
		errorCase(!colValue);
		errorCase(!isJLine(colValue, 1, 0, 0, 1));
	}
}
static void CheckColNames_2(autoList_t *colNames, autoList_t *tableColNames)
{
	char *colName;
	uint colidx;

	foreach (colNames, colName, colidx)
	{
		errorCase(
			findJLineICase(tableColNames, colName) == getCount(tableColNames) &&
			strcmp("$", colName)
			);
	}
}
static autoList_t *GetIndexList(autoList_t *colNames, autoList_t *tableColNames)
{
	autoList_t *result = newList();
	char *colName;
	uint colidx;

	foreach (colNames, colName, colidx)
	{
		addElement(result, findJLineICase(tableColNames, colName));
	}
	return result;
}

autoList_t *TR_GetSchema(char *csvFile)
{
	FILE *fp;
	autoList_t *colNames;

	errorCase(isEmptyJTkn(csvFile));

	fp = fileOpen(csvFile, "rt");
	colNames = CSVStreamToRow(fp);

	CheckColNames(colNames);

	fileClose(fp);
	return colNames;
}
autoList_t *TR_SelectTop1WhereAnd(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, autoList_t *retColNames) // ret: NULL == not found
{
	FILE *fp;
	autoList_t *colNames;
	autoList_t *whereIndexes;
	autoList_t *retIndexes;
	autoList_t *row;
	uint colidx;
	uint index;

	errorCase(isEmptyJTkn(csvFile));
	CheckColNames(whereColNames);
	CheckColValues(whereColValues, getCount(whereColNames));
	CheckColNames(retColNames);

	fp = fileOpen(csvFile, "rt");
	colNames = CSVStreamToRow(fp);

	CheckColNames(colNames);
	CheckColNames_2(whereColNames, colNames);
	CheckColNames_2(retColNames, colNames);

	whereIndexes = GetIndexList(whereColNames, colNames);
	retIndexes = GetIndexList(retColNames, colNames);

	while (row = CSVStreamToRow(fp))
	{
		foreach (whereIndexes, colidx, index)
			if (strcmp(refLine(row, colidx), getLine(whereColValues, index))) // ? not match
				break;

		if (index == getCount(whereIndexes)) // ? match
			break;

		releaseDim(row, 1);
	}
	fileClose(fp);

	if (row) // ? found
	{
		autoList_t *row2 = newList();

		foreach (retIndexes, colidx, index)
			addElement(row2, (uint)strx(refLine(row, colidx)));

		releaseDim(row, 1);
		row = row2;
	}
	return row;
}
uint64 TR_SelectWhereAnd(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, autoList_t *destColNames, char *destFile) // ret: 発見した行数
{
	FILE *fp;
	FILE *destfp;
	autoList_t *colNames;
	autoList_t *whereIndexes;
	autoList_t *destIndexes;
	autoList_t *row;
	uint colidx;
	uint index;
	uint64 fndnum = 0;

	errorCase(isEmptyJTkn(csvFile));
	CheckColNames(whereColNames);
	CheckColValues(whereColValues, getCount(whereColNames));
	CheckColNames(destColNames);
	errorCase(isEmptyJTkn(destFile)); // csvFileと同じパスは不可

	fp = fileOpen(csvFile, "rt");
	colNames = CSVStreamToRow(fp);

	CheckColNames(colNames);
	CheckColNames_2(whereColNames, colNames);
	CheckColNames_2(destColNames, colNames);

	whereIndexes = GetIndexList(whereColNames, colNames);
	destIndexes = GetIndexList(destColNames, colNames);
	destfp = fileOpen(destFile, "wt");
	releaseDim(colNames, 1);

	while (row = CSVStreamToRow(fp))
	{
		foreach (whereIndexes, colidx, index)
			if (strcmp(refLine(row, colidx), getLine(whereColValues, index))) // ? not match
				break;

		if (index == getCount(whereIndexes)) // ? match
		{
			foreach (destIndexes, colidx, index)
				writeLine(destfp, refLine(row, colidx));

			fndnum++;
		}
		releaseDim(row, 1);
	}
	fileClose(fp);
	fileClose(destfp);

	return fndnum;
}
uint64 TR_DeleteWhereAnd(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, char *destCsvFile) // ret: 削除した行数
{
	FILE *fp;
	FILE *destfp;
	autoList_t *colNames;
	autoList_t *whereIndexes;
	autoList_t *row;
	uint colidx;
	uint index;
	uint64 delnum = 0;

	errorCase(isEmptyJTkn(csvFile));
	CheckColNames(whereColNames);
	CheckColValues(whereColValues, getCount(whereColNames));
	errorCase(isEmptyJTkn(destCsvFile)); // csvFileと同じパスは不可

	fp = fileOpen(csvFile, "rt");
	colNames = CSVStreamToRow(fp);

	CheckColNames(colNames);
	CheckColNames_2(whereColNames, colNames);

	whereIndexes = GetIndexList(whereColNames, colNames);
	destfp = fileOpen(destCsvFile, "wt");
	RowToCSVStream(destfp, colNames);
	releaseDim(colNames, 1);

	while (row = CSVStreamToRow(fp))
	{
		foreach (whereIndexes, colidx, index)
			if (strcmp(refLine(row, colidx), getLine(whereColValues, index))) // ? not match
				break;

		if (index == getCount(whereIndexes)) // ? match
			delnum++;
		else
			RowToCSVStream(destfp, row);

		releaseDim(row, 1);
	}
	fileClose(fp);
	fileClose(destfp);

	return delnum;
}
void TR_InsertInto(char *csvFile, autoList_t *colNames, autoList_t *colValueTable, char *destCsvFile)
{
	FILE *fp;
	FILE *destfp;
	autoList_t *tableColNames;
	autoList_t *destIndexes;
	autoList_t *row;
	uint rowidx;
	uint colidx;
	uint index;

	errorCase(isEmptyJTkn(csvFile));
	CheckColNames(colNames);
	errorCase(!colValueTable);
	errorCase(isEmptyJTkn(destCsvFile)); // csvFileと同じパスは不可

	fp = fileOpen(csvFile, "rt");
	tableColNames = CSVStreamToRow(fp);

	CheckColNames(tableColNames);
	CheckColNames_2(colNames, tableColNames);

	destIndexes = GetIndexList(colNames, tableColNames);
	destfp = fileOpen(destCsvFile, "wt");
	RowToCSVStream(destfp, colNames);
	releaseDim(tableColNames, 1);

	while (row = CSVStreamToRow(fp))
	{
		TrimCSVRow(row, getCount(colNames));
		RowToCSVStream(destfp, row);
		releaseDim(row, 1);
	}
	fileClose(fp);

	foreach (colValueTable, row, rowidx)
	{
		autoList_t *row2 = newList();

		CheckColValues(row, getCount(colNames));

		foreach (destIndexes, colidx, index)
			addElement(row2, (uint)refLine(row, colidx));

		RowToCSVStream(destfp, row2);
		releaseAutoList(row2);
	}
	fileClose(destfp);
}
void TR_CreateTable(char *csvFile, autoList_t *colNames)
{
	FILE *fp;

	errorCase(isEmptyJTkn(csvFile));
	CheckColNames(colNames);

	fp = fileOpen(csvFile, "wt");
	RowToCSVStream(fp, colNames);
	fileClose(fp);
}

// _x
autoList_t *TR_SelectTop1WhereAnd_cxxx(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, autoList_t *retColNames)
{
	autoList_t *retval = TR_SelectTop1WhereAnd(csvFile, whereColNames, whereColValues, retColNames);
	releaseDim(whereColNames, 1);
	releaseDim(whereColValues, 1);
	releaseDim(retColNames, 1);
	return retval;
}
uint64 TR_SelectWhereAnd_cxxxc(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, autoList_t *destColNames, char *destFile)
{
	uint64 retval = TR_SelectWhereAnd(csvFile, whereColNames, whereColValues, destColNames, destFile);
	releaseDim(whereColNames, 1);
	releaseDim(whereColValues, 1);
	releaseDim(destColNames, 1);
	return retval;
}
uint64 TR_DeleteWhereAnd_cxxc(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, char *destCsvFile)
{
	uint64 retval = TR_DeleteWhereAnd(csvFile, whereColNames, whereColValues, destCsvFile);
	releaseDim(whereColNames, 1);
	releaseDim(whereColValues, 1);
	return retval;
}
void TR_InsertInto_cxcc(char *csvFile, autoList_t *colNames, autoList_t *colValueTable, char *destCsvFile)
{
	TR_InsertInto(csvFile, colNames, colValueTable, destCsvFile);
	releaseDim(colNames, 1);
}
void TR_InsertInto_cxxc(char *csvFile, autoList_t *colNames, autoList_t *colValueTable, char *destCsvFile)
{
	TR_InsertInto_cxcc(csvFile, colNames, colValueTable, destCsvFile);
	releaseDim(colValueTable, 2);
}
void TR_CreateTable_cx(char *csvFile, autoList_t *colNames)
{
	TR_CreateTable(csvFile, colNames);
	releaseDim(colNames, 1);
}
