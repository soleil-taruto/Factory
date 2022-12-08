/*
	SQLっぽいやつ

		SELECT * FROM [TABLE] WHERE [C-COLUMN] = [C-VALUE]

		SELECT [R-COLUMN] [R-COLUMN] [R-COLUMN] ... FROM [TABLE] WHERE [C-COLUMN] = [C-VALUE]

		INSERT INTO [TABLE] ( [W-COLUMN] [W-COLUMN] [W-COLUMN] ... ) VALUES ( [W-VALUE] [W-VALUE] [W-VALUE] ... )

		UPDATE [TABLE] SET [W-COLUMN] = [W-VALUE] ... WHERE [C-COLUMN] = [C-VALUE]

		DELETE FROM [TABLE] WHERE [C-COLUMN] = [C-VALUE]

	１件だけ処理

		LGET [TABLE] [C-COLUMN] [C-VALUE] [R-COLUMN]

		LSET [TABLE] [C-COLUMN] [C-VALUE] [W-COLUMN] [W-VALUE]

		RGET [TABLE] [ROW] [R-COLUMN]

		RSET [TABLE] [ROW] [W-COLUMN] [W-VALUE]

	その他

		MKID

		DELTBL [TABLE]

		DELCOL [TABLE] [COLUMN]

	構造を返す。

		TBLS

		COLS [TABLE]

		ROWS [TABLE]

	----

	不正なクエリ -> error();
*/

#include "Query.h"

static char *QryRdr;
static int Quoted;

static int IsBlank(int chr)
{
	return chr <= ' ' || chr == '(' || chr == ')' || chr == ',';
}
static char *TryNextQryToken(void)
{
	autoBlock_t *buff;

	Quoted = 0;

	for (; IsBlank(*QryRdr); QryRdr++)
		if (*QryRdr == '\0')
			return NULL;

	buff = newBlock();

	if (*QryRdr == '\'')
	{
		Quoted = 1;

		for (; ; )
		{
			int chr = *++QryRdr;

			errorCase(chr == '\0');

			if (chr == '\'')
			{
				QryRdr++;
				break;
			}
			if (chr == '\\')
			{
				chr = *++QryRdr;

				switch (chr)
				{
				case '\0':
					error();

				case 't': chr = '\t'; break;
				case 'n': chr = '\n'; break;
				case 'r': chr = '\r'; break;
				case 's': chr = ' '; break;
				case 'x':
					{
						int chr1 = *++QryRdr;
						int chr2;

						errorCase(!m_ishexadecimal(chr1));
						chr2 = *++QryRdr;
						errorCase(!m_ishexadecimal(chr2));

						chr = c2i(chr1) * 16 + c2i(chr2);

						errorCase(chr == '\0');
					}
					break;

				default:
					break;
				}
			}
			else if (isMbc(QryRdr))
			{
				addByte(buff, chr);
				chr = *++QryRdr;
			}
			addByte(buff, chr);
		}
	}
	else
	{
		for (; !IsBlank(*QryRdr); QryRdr++)
			addByte(buff, *QryRdr);
	}
	return unbindBlock2Line(buff);
}
char *FC_Retoken(char *token) // ret: strx()
{
	autoBlock_t *buff = newBlock();
	char *p;

	for (p = token; *p; p++)
	{
		if (isJCharP(p))
		{
			addByte(buff, *p++);
			addByte(buff, *p);
		}
		else if (*p == '\\')
		{
			addByte(buff, '\\');
			addByte(buff, '\\');
		}
		else if (m_isasciikana(*p))
		{
			addByte(buff, *p);
		}
		else
		{
			addByte(buff, '\\');
			addByte(buff, 'x');
			addByte(buff, hexadecimal[(uint)*p / 16]);
			addByte(buff, hexadecimal[(uint)*p % 16]);
		}
	}
	return unbindBlock2Line(buff);
}
static char *NextQryToken(void)
{
	char *ret = TryNextQryToken();
	errorCase(!ret);
	return ret;
}
static int NextQryTokenIs(char *spell)
{
	char *token = NextQryToken();
	int ret;

	if (token)
	{
		ret = !_stricmp(token, spell);
		memFree(token);
	}
	else
	{
		ret = 0;
	}
	return ret;
}

static autoList_t *Ret;

static void ExecuteSelect(void)
{
	autoList_t *retColumns = newList();
	int starColumn = 0;
	char *table;
	char *whereColumn;
	char *whereValue;
	autoList_t *rowIds;
	char *rowId;
	uint rowidx;

	for (; ; )
	{
		char *column = NextQryToken();

		if (!Quoted)
		{
			if (!strcmp(column, "*"))
			{
				starColumn = 1;
				memFree(column);
				continue;
			}
			if (!_stricmp(column, "FROM"))
			{
				memFree(column);
				break;
			}
		}
		addElement(retColumns, (uint)column);
	}
	table = NextQryToken();
	errorCase(!NextQryTokenIs("WHERE"));
	whereColumn = NextQryToken();
	errorCase(!NextQryTokenIs("="));
	whereValue = NextQryToken();
	errorCase(TryNextQryToken());

	if (starColumn)
	{
		addElements_x(retColumns, FC_GetAllColumnId(table));
	}
	rowIds = FC_GetStrRowIds(table, whereColumn, whereValue);

	foreach (rowIds, rowId, rowidx)
	{
		autoList_t *row = newList();
		char *column;
		uint colidx;

		foreach (retColumns, column, colidx)
		{
			addElement(row, (uint)FC_GetStrValue(table, rowId, column));
		}
		addElement(Ret, (uint)row);
	}
	releaseDim(retColumns, 1);
	memFree(table);
	memFree(whereColumn);
	memFree(whereValue);
}
static void ExecuteInsert(void)
{
	char *table;
	autoList_t *insColumns = newList();
	autoList_t *insValues  = newList();
	char *rowId;
	char *column;
	uint colidx;

	errorCase(!NextQryTokenIs("INTO"));
	table = NextQryToken();

	for (; ; )
	{
		column = NextQryToken();

		if (!Quoted && !_stricmp(column, "VALUES"))
		{
			memFree(column);
			break;
		}
		addElement(insColumns, (uint)column);
	}
	for (colidx = 0; colidx < getCount(insColumns); colidx++)
	{
		char *value = NextQryToken();

		addElement(insValues, (uint)value);
	}
	errorCase(TryNextQryToken());

	rowId = FC_GetNewId();

	foreach (insColumns, column, colidx)
	{
		FC_SetStrValue(table, rowId, column, getLine(insValues, colidx));
	}
	memFree(table);
	releaseDim(insColumns, 1);
	releaseDim(insValues, 1);
	memFree(rowId);
}
static void ExecuteUpdate(void)
{
	char *table;
	autoList_t *updColumns = newList();
	autoList_t *updValues  = newList();
	char *whereColumn;
	char *whereValue;
	autoList_t *rowIds;
	char *rowId;
	uint rowidx;
	char *column;
	uint colidx;

	table = NextQryToken();
	errorCase(!NextQryTokenIs("SET"));

	for (; ; )
	{
		char *column = NextQryToken();

		if (!Quoted && !_stricmp(column, "WHERE"))
		{
			memFree(column);
			break;
		}
		addElement(updColumns, (uint)column);
		errorCase(!NextQryTokenIs("="));
		addElement(updValues, (uint)NextQryToken());
	}
	whereColumn = NextQryToken();
	errorCase(!NextQryTokenIs("="));
	whereValue = NextQryToken();
	errorCase(TryNextQryToken());

	rowIds = FC_GetStrRowIds(table, whereColumn, whereValue);

	foreach (rowIds, rowId, rowidx)
	foreach (updColumns, column, colidx)
	{
		FC_SetStrValue(table, rowId, column, getLine(updValues, colidx));
	}
	memFree(table);
	releaseDim(updColumns, 1);
	releaseDim(updValues, 1);
	memFree(whereColumn);
	memFree(whereValue);
	releaseDim(rowIds, 1);
}
static void ExecuteDelete(void)
{
	char *table;
	char *whereColumn;
	char *whereValue;
	autoList_t *rowIds;
	char *rowId;
	uint rowidx;

	errorCase(!NextQryTokenIs("FROM"));
	table = NextQryToken();
	errorCase(!NextQryTokenIs("WHERE"));
	whereColumn = NextQryToken();
	errorCase(!NextQryTokenIs("="));
	whereValue = NextQryToken();
	errorCase(TryNextQryToken());

	rowIds = FC_GetStrRowIds(table, whereColumn, whereValue);

	foreach (rowIds, rowId, rowidx)
	{
		FC_DeleteRow(table, rowId);
	}
	memFree(table);
	memFree(whereColumn);
	memFree(whereValue);
	releaseDim(rowIds, 1);
}
static void ExecuteLGet(void)
{
	char *table;
	char *whereColumn;
	char *whereValue;
	char *retColumn;
	char *rowId;
	char *value;
	autoList_t *row = newList();

	table       = NextQryToken();
	whereColumn = NextQryToken();
	whereValue  = NextQryToken();
	retColumn   = NextQryToken();

	errorCase(TryNextQryToken());

	rowId = FC_GetStrRowId(table, whereColumn, whereValue);
	value = FC_GetStrValue(table, rowId, retColumn);

	addElement(row, (uint)value);
	addElement(Ret, (uint)row);

	memFree(table);
	memFree(whereColumn);
	memFree(whereValue);
	memFree(retColumn);
	memFree(rowId);
}
static void ExecuteLSet(void)
{
	char *table;
	char *whereColumn;
	char *whereValue;
	char *updColumn;
	char *updValue;
	char *rowId;

	table       = NextQryToken();
	whereColumn = NextQryToken();
	whereValue  = NextQryToken();
	updColumn   = NextQryToken();
	updValue    = NextQryToken();

	errorCase(TryNextQryToken());

	rowId = FC_GetStrRowId(table, whereColumn, whereValue);
	FC_SetStrValue(table, rowId, updColumn, updValue);

	memFree(table);
	memFree(whereColumn);
	memFree(whereValue);
	memFree(updColumn);
	memFree(updValue);
	memFree(rowId);
}
static void ExecuteRGet(void)
{
	char *table;
	char *rowNameOrId;
	char *retColumn;
	char *value;
	autoList_t *row = newList();

	table       = NextQryToken();
	rowNameOrId = NextQryToken();
	retColumn   = NextQryToken();

	errorCase(TryNextQryToken());

	value = FC_GetStrValue(table, rowNameOrId, retColumn);

	addElement(row, (uint)value);
	addElement(Ret, (uint)row);

	memFree(table);
	memFree(rowNameOrId);
	memFree(retColumn);
}
static void ExecuteRSet(void)
{
	char *table;
	char *rowNameOrId;
	char *updColumn;
	char *updValue;

	table       = NextQryToken();
	rowNameOrId = NextQryToken();
	updColumn   = NextQryToken();
	updValue    = NextQryToken();

	errorCase(TryNextQryToken());

	FC_SetStrValue(table, rowNameOrId, updColumn, updValue);

	memFree(table);
	memFree(rowNameOrId);
	memFree(updColumn);
	memFree(updValue);
}
static void ExecuteMkId(void)
{
	char *rowId;
	autoList_t *row = newList();

	errorCase(TryNextQryToken());

	rowId = FC_GetNewId();

	addElement(row, (uint)rowId);
	addElement(Ret, (uint)row);
}
static void ExecuteDelTbl(void)
{
	char *table;

	table = NextQryToken();

	errorCase(TryNextQryToken());

	FC_DeleteTable(table);

	memFree(table);
}
static void ExecuteDelCol(void)
{
	char *table;
	char *column;

	table  = NextQryToken();
	column = NextQryToken();

	errorCase(TryNextQryToken());

	FC_DeleteColumn(table, column);

	memFree(table);
	memFree(column);
}
static void ExecuteTbls(void)
{
	autoList_t *tableIds;
	char *tableId;
	uint index;

	errorCase(TryNextQryToken());

	tableIds = FC_GetAllTableId();

	foreach (tableIds, tableId, index)
	{
		autoList_t *row = newList();

		addElement(row, (uint)tableId);
		addElement(Ret, (uint)row);
	}
	releaseAutoList(tableIds);
}
static void ExecuteCols(void)
{
	char *table;
	autoList_t *columnIds;
	char *columnId;
	uint index;

	table = NextQryToken();

	errorCase(TryNextQryToken());

	columnIds = FC_GetAllColumnId(table);

	foreach (columnIds, columnId, index)
	{
		autoList_t *row = newList();

		addElement(row, (uint)columnId);
		addElement(Ret, (uint)row);
	}
	memFree(table);
	releaseAutoList(columnIds);
}
static void ExecuteRows(void)
{
	char *table;
	autoList_t *rowIds;
	char *rowId;
	uint index;

	table = NextQryToken();

	errorCase(TryNextQryToken());

	rowIds = FC_GetTableAllRowId(table);

	foreach (rowIds, rowId, index)
	{
		autoList_t *row = newList();

		addElement(row, (uint)rowId);
		addElement(Ret, (uint)row);
	}
	memFree(table);
	releaseAutoList(rowIds);
}
autoList_t *FC_ExecuteQuery(char *query) // ret: bind
{
	char *method;

	errorCase(!query);

	QryRdr = query;
	method = NextQryToken();

	if (Ret)
		releaseDim(Ret, 2);

	Ret = newList();

	if (!_stricmp(method, "SELECT"))
	{
		ExecuteSelect();
	}
	else if (!_stricmp(method, "INSERT"))
	{
		ExecuteInsert();
	}
	else if (!_stricmp(method, "UPDATE"))
	{
		ExecuteUpdate();
	}
	else if (!_stricmp(method, "DELETE"))
	{
		ExecuteDelete();
	}
	else if (!_stricmp(method, "LGET"))
	{
		ExecuteLGet();
	}
	else if (!_stricmp(method, "LSET"))
	{
		ExecuteLSet();
	}
	else if (!_stricmp(method, "RGET"))
	{
		ExecuteRGet();
	}
	else if (!_stricmp(method, "RSET"))
	{
		ExecuteRSet();
	}
	else if (!_stricmp(method, "MKID"))
	{
		ExecuteMkId();
	}
	else if (!_stricmp(method, "DELTBL"))
	{
		ExecuteDelTbl();
	}
	else if (!_stricmp(method, "DELCOL"))
	{
		ExecuteDelCol();
	}
	else if (!_stricmp(method, "TBLS"))
	{
		ExecuteTbls();
	}
	else if (!_stricmp(method, "COLS"))
	{
		ExecuteCols();
	}
	else if (!_stricmp(method, "ROWS"))
	{
		ExecuteRows();
	}
	else
	{
		error();
	}
	memFree(method);
	QryRdr = NULL;
	return Ret;
}

// _x
char *FC_Retoken_x(char *token)
{
	char *ret = FC_Retoken(token);
	memFree(token);
	return ret;
}
autoList_t *FC_ExecuteQuery_x(char *query)
{
	autoList_t *ret = FC_ExecuteQuery(query);
	memFree(query);
	return ret;
}
