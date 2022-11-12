/*
	[エラーチェックの方針]

	ファイル名
	テーブル名
	カラム名
		日本語文字列であること。
		タブ・改行共に不可
		空白での開始終了・空文字列共に不可
	フィールド値
		日本語文字列であること。
		タブ・改行共に不可
		空白での開始終了・空文字列共に可

	カラム
		1 以上
	レコード数
		0 以上

	DBとの整合性のチェックは行わない。

	TA_User
	TA_Pass
	TA_DBName
		ノーチェック
*/

#include "TableAccessor.h"

char *TA_User; // NULL == 省略
char *TA_Pass; // NULL == 省略
char *TA_DBName = "DWDB"; // NULL不可

static void ExecSqlFile(char *sqlFile, char *redirFile)
{
	char *command = strx("sqlcmd -k");

	if (TA_User)
		command = addLine_x(command, xcout(" -U \"%s\"", TA_User));

	if (TA_Pass)
		command = addLine_x(command, xcout(" -P \"%s\"", TA_Pass));

	command = addLine_x(command, xcout(" -i \"%s\"", sqlFile));

	if (redirFile)
		command = addLine_x(command, xcout(" -o \"%s\"", redirFile));

	execute(command);
	memFree(command);
}
static void ExecSql(char *sql, char *redirFile)
{
	char *sqlFile = makeTempFile("sql");

	{
		FILE *fp = fileOpen(sqlFile, "wt");

		writeLine_x(fp, xcout("USE %s", TA_DBName));
		writeLine(fp, "GO");
		writeLine(fp, sql);

		fileClose(fp);
	}

	ExecSqlFile(sqlFile, redirFile);

	removeFile(sqlFile);
	memFree(sqlFile);
}
static void ExecSql_xc(char *sql, char *redirFile)
{
	ExecSql(sql, redirFile);
	memFree(sql);
}

void TA_GetTableData_R(char *tableName, char *redirFile)
{
	errorCase(isEmptyJTkn(tableName));
	errorCase(isEmptyJTkn(redirFile)); // 出力先ファイル

	ExecSql_xc(xcout("SELECT * FROM [%s]", tableName), redirFile);
}
void TA_GetTableSchema_R(char *tableName, char *redirFile)
{
	errorCase(isEmptyJTkn(tableName));
	errorCase(isEmptyJTkn(redirFile)); // 出力先ファイル

	ExecSql_xc(xcout("sp_columns [%s]", tableName), redirFile);
}
void TA_GetTableList_R(char *redirFile)
{
	TA_GetTableData_R("SYS].[TABLES", redirFile);
}

static void GetTableData(char *tableName, char *csvFile, void (*tableDataGetter)(char *, char *))
{
	char *midFile = makeTempFile("sqlcmd-out");

	tableDataGetter(tableName, midFile);
	RSTR_ToCSVFile(midFile, csvFile);

	removeFile(midFile);
	memFree(midFile);
}

void TA_GetTableData(char *tableName, char *csvFile)
{
	errorCase(isEmptyJTkn(tableName));
	errorCase(isEmptyJTkn(csvFile)); // 出力先CSVファイル

	GetTableData(tableName, csvFile, TA_GetTableData_R);
}
void TA_GetTableSchema(char *tableName, char *csvFile)
{
	errorCase(isEmptyJTkn(tableName));
	errorCase(isEmptyJTkn(csvFile)); // 出力先CSVファイル

	GetTableData(tableName, csvFile, TA_GetTableSchema_R);
}
void TA_GetTableList(char *csvFile)
{
	TA_GetTableData("SYS].[TABLES", csvFile);
}

static char *ValueFltr(char *value)
{
	return replaceLine(strx(value), "\'", "\'\'", 0);
}
static char *c_ValueFltr(char *value)
{
	static char *stock;
	memFree(stock);
	return stock = ValueFltr(value);
}
static char *GetValueList(autoList_t *values, char *kakko)
{
	char *result = strx("");
	char *value;
	uint colidx;

	foreach (values, value, colidx)
	{
		if (colidx)
			result = addLine(result, ", ");

		result = addChar(result, kakko[0]);
		result = addLine(result, c_ValueFltr(value));
		result = addChar(result, kakko[1]);
	}
	return result;
}
static char *GetKeyEqValueAndList(autoList_t *keys, autoList_t *values)
{
	char *result = strx("");
	char *value;
	uint colidx;

	foreach (values, value, colidx)
	{
		if (colidx)
			result = addLine(result, " AND ");

		result = xcout("[%s] = '%s'", getLine(keys, colidx), c_ValueFltr(value));
	}
	return result;
}

static void CheckTableInput(char *tableName, autoList_t *colNames, autoList_t *rows)
{
	// tableName
	{
		errorCase(!tableName);
		errorCase(*tableName == '\0');
		errorCase(lineExp(" <>", tableName));
		errorCase(lineExp("<> ", tableName));
		errorCase(!isJLine(tableName, 1, 0, 0, 1));
	}
	// colNames
	{
		char *colName;
		uint colidx;

		errorCase(!colNames);
		errorCase(getCount(colNames) < 1);

		foreach (colNames, colName, colidx)
		{
			errorCase(!colName);
			errorCase(*colName == '\0');
			errorCase(lineExp(" <>", colName));
			errorCase(lineExp("<> ", colName));
			errorCase(!isJLine(colName, 1, 0, 0, 1));
		}
	}
	// rows
	{
		autoList_t *row;
		uint rowidx;

		errorCase(!rows);

		foreach (rows, row, rowidx)
		{
			char *cell;
			uint colidx;

			errorCase(!row);
			errorCase(getCount(row) != getCount(colNames));

			foreach (row, cell, colidx)
			{
				errorCase(!cell);
				errorCase(!isJLine(cell, 1, 0, 0, 1));
			}
		}
	}
}

void TA_InsertMultiRow(char *tableName, autoList_t *colNames, autoList_t *rows)
{
	char *file;
	FILE *fp;
	char *insh;
	autoList_t *row;
	uint rowidx;
	char *sql;

	CheckTableInput(tableName, colNames, rows);

	file = makeTempFile("sql");
	fp = fileOpen(file, "wt");

	writeLine_x(fp, xcout("USE %s", TA_DBName));
	writeLine(fp, "GO");

	insh = xcout("INSERT INTO [%s] (", tableName);
	insh = addLine_x(insh, GetValueList(colNames, "[]"));
	insh = addLine(insh, ") VALUES (");

	foreach (rows, row, rowidx)
	{
		sql = strx(insh);
		sql = addLine_x(sql, GetValueList(row, "''"));
		sql = addChar(sql, ')');

		writeLine_x(fp, sql);
	}
	memFree(insh);
	fileClose(fp);

	ExecSqlFile(file, NULL);

	removeFile(file);
	memFree(file);
}
void TA_DeleteMultiRow(char *tableName, autoList_t *colNames, autoList_t *rows)
{
	char *file;
	FILE *fp;
	char *insh;
	autoList_t *row;
	uint rowidx;
	char *sql;

	CheckTableInput(tableName, colNames, rows);

	file = makeTempFile("sql");
	fp = fileOpen(file, "wt");

	writeLine_x(fp, xcout("USE %s", TA_DBName));
	writeLine(fp, "GO");

	insh = xcout("DELETE FROM [%s] WHERE ", tableName);

	foreach (rows, row, rowidx)
	{
		sql = strx(insh);
		sql = addLine_x(sql, GetKeyEqValueAndList(colNames, row));

		writeLine_x(fp, sql);
	}
	memFree(insh);
	fileClose(fp);

	ExecSqlFile(file, NULL);

	removeFile(file);
	memFree(file);
}

void TA_InsertRow(char *tableName, autoList_t *colNames, autoList_t *row)
{
	autoList_t *rows = newList();

	addElement(rows, (uint)row);
	TA_InsertMultiRow(tableName, colNames, rows);
	releaseAutoList(rows);
}
void TA_DeleteRow(char *tableName, autoList_t *colNames, autoList_t *row)
{
	autoList_t *rows = newList();

	addElement(rows, (uint)row);
	TA_DeleteMultiRow(tableName, colNames, rows);
	releaseAutoList(rows);
}

static void BulkInsert(char *tableName, autoList_t *colNames, autoList_t *rows)
{
	char *csvFile;
	char *sqlFile;
	FILE *fp;
	autoList_t *row;
	char *colName;
	uint rowidx;
	uint colidx;

	CheckTableInput(tableName, colNames, rows);

	if (getCount(rows) < 1)
		return;

	csvFile = makeTempFile("csv");
	sqlFile = makeTempFile("sql");

	fp = fileOpen(csvFile, "wt");

	foreach (rows, row, rowidx)
	{
		RowToCSVStream(fp, row);
	}
	fileClose(fp);

	fp = fileOpen(sqlFile, "wt");

	writeLine_x(fp, xcout("USE %s", TA_DBName));
	writeLine(fp, "GO");
	writeLine_x(fp, xcout("BULK INSERT [%s]", tableName));
	writeLine_x(fp, xcout("FROM '%s'", csvFile));
	writeLine(fp, "WITH");
	writeLine(fp, "(");
	writeLine(fp, "\tFIELDTERMINATOR = ',',");
	writeLine(fp, "\tROWTERMINATOR = '\\n',");
	writeLine(fp, "\tORDER");
	writeLine(fp, "\t(");

	foreach (colNames, colName, colidx)
	{
		writeLine_x(fp, xcout("\t\t[%s]%s", colName, colidx + 1 < getCount(colNames) ? "," : ""));
	}
	writeLine(fp, "\t)");
	writeLine(fp, ");");

	fileClose(fp);

	ExecSqlFile(sqlFile, NULL);

	removeFile(csvFile);
	removeFile(sqlFile);
	memFree(csvFile);
	memFree(sqlFile);
}
static void ExecCSVFile(
	void (*execMultiRow)(char *, autoList_t *, autoList_t *),
	char *tableName, autoList_t *colNames, char *csvFile, uint rowcntPerExec, uint rdszPerExec
	)
{
	FILE *fp;
	autoList_t *row;
	autoList_t *rows = newList();
	uint hfc = 0;
	uint64 execrowcnt = 0;
	uint64 lastseek = 0;
	uint rowcnt;

	errorCase(isEmptyJTkn(csvFile));

	if (!rowcntPerExec)
		rowcntPerExec = 100 * 1000 * 1000; // デフォルト

	if (!rdszPerExec)
		rdszPerExec = 100 * 1000 * 1000; // デフォルト

	// tableName, colNames, csvFile(の内容) のエラーチェックは execMultiRow に任せる、この関数内ではスルー。

	fp = fileOpen(csvFile, "rt");

	if (!colNames)
	{
		colNames = CSVStreamToRow(fp);
		errorCase(!colNames);
		TrimCSVRow(colNames, 0);
		hfc = 1;
	}
	while (row = CSVStreamToRow(fp))
	{
		TrimCSVRow(row, getCount(colNames));
		addElement(rows, (uint)row);
		rowcnt = getCount(rows);

		if (rowcntPerExec <= rowcnt || (rowcnt < 1000 || rowcnt % 1000 == 0) && lastseek + (uint64)rdszPerExec <= (uint64)_ftelli64(fp))
		{
			execMultiRow(tableName, colNames, rows);
			execrowcnt += getCount(rows);
			lastseek = (uint64)_ftelli64(fp);
			releaseDim(rows, 2);

			rows = newList();
			cout("(%u 行処理しました...継続します)\n", execrowcnt);
		}
	}
	execMultiRow(tableName, colNames, rows);
	execrowcnt += getCount(rows);
	releaseDim(rows, 2);

	cout("(%u 行処理しました...終了しました)\n", execrowcnt);

	if (hfc)
		releaseDim(colNames, 1);
}
void TA_InsertCSVFile(char *tableName, autoList_t *colNames, char *csvFile, uint rowcntPerExec, uint rdszPerExec)
{
	ExecCSVFile(BulkInsert, tableName, colNames, csvFile, rowcntPerExec, rdszPerExec);
}
void TA_DeleteCSVFile(char *tableName, autoList_t *colNames, char *csvFile, uint rowcntPerExec, uint rdszPerExec)
{
	ExecCSVFile(TA_DeleteMultiRow, tableName, colNames, csvFile, rowcntPerExec, rdszPerExec);
}
void TA_DeleteAll(char *tableName)
{
	errorCase(isEmptyJTkn(tableName));
	ExecSql_xc(xcout("DELETE FROM [%s]", tableName), NULL);
}

void TA_TableDataFltr(char *csvFile, autoList_t *colNames, char *outCsvFile)
{
	FILE *fp;
	FILE *outfp;
	autoList_t *colodr = newList();
	autoList_t *row;
	char *colName;
	uint colidx;
	uint colidx2;
	autoList_t *row2 = newList();

	errorCase(isEmptyJTkn(csvFile));
	errorCase(isEmptyJTkn(outCsvFile));

	{
		static autoList_t *DUMMY_ROWS;

		if (!DUMMY_ROWS)
			DUMMY_ROWS = newList();

		CheckTableInput("DUMMY_TABLE_NAME", colNames, DUMMY_ROWS);
	}

	fp = fileOpen(csvFile, "rt");
	outfp = fileOpen(outCsvFile, "wt");

	row = CSVStreamToRow(fp);
	errorCase(!row);

	foreach (colNames, colName, colidx)
	{
		addElement(colodr, findJLineICase(row, colName));
	}
	releaseDim(row, 1);

	while (row = CSVStreamToRow(fp))
	{
		foreach (colodr, colidx2, colidx)
		{
			addElement(row2, (uint)refLine(row, colidx2));
		}
		RowToCSVStream(outfp, row2);

		releaseDim(row, 1);
		setCount(row2, 0);
	}
	releaseAutoList(colodr);
	releaseAutoList(row2);

	fileClose(fp);
	fileClose(outfp);
}
void TA_TableDataFltr_cxc(char *csvFile, autoList_t *colNames, char *outCsvFile)
{
	TA_TableDataFltr(csvFile, colNames, outCsvFile);
	releaseDim(colNames, 1);
}
