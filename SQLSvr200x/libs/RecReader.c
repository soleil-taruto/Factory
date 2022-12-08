/*
	改行を含むレコードを想定しない。
	sqlcmd 実行時に -k2 を付けること！

	ex.
		sqlcmd -S SERVER -k2 -i SCRIPT.sql > OUT.txt
*/

#include "RecReader.h"

static autoList_t *GetColumnWidthList(char *bdrLine)
{
	autoList_t *cwList = newList();
	char *p;
	uint cw = 0;

	errorCase(
		!lineExp("<  -->", bdrLine) ||
		!lineExp("-<>-", bdrLine) ||
		lineExp("<>  <>", bdrLine)
		);

	for (p = bdrLine; *p; p++)
	{
		if (*p == ' ')
		{
			addElement(cwList, cw);
			cw = 0;
		}
		else
		{
			cw++;
		}
	}
	addElement(cwList, cw);

	// check
	{
		uint colidx;

		errorCase(getCount(cwList) < 1); // ? 列が１つも無い。

		for (colidx = 0; colidx < getCount(cwList); colidx++)
		{
			errorCase(getElement(cwList, colidx) < 1); // ? 列幅が１文字も無い。
		}
	}
	return cwList;
}
static autoList_t *SplitRow(char *recLine, autoList_t *cwList)
{
	autoList_t *row = newList();
	uint c = getElement(cwList, 0);
	char *column = strx("");
	char *p = recLine;

	for (; ; )
	{
		if (!c)
		{
			addElement(row, (uint)column);

			if (getCount(cwList) == getCount(row))
				break;

			c = getElement(cwList, getCount(row));
			column = strx("");

			errorCase(*p++ != ' '); // 列の区切りが空白ではない。
		}
		errorCase(!*p); // 行の途中で終了

		if (isMbc(p))
		{
			column = addChar(column, *p++);
		}
		column = addChar(column, *p++);
		c--;
	}
	errorCase(*p); // 行の終端で終了しない。

	return row;
}
autoList_t *SqlRecReader(char *recFile)
{
	autoList_t *recLines;
	autoList_t *cwList;
	char *recLine;
	uint rowidx;
	autoList_t *table = newList();
	autoList_t *row;
	char *column;
	uint colidx;

	errorCase(m_isEmpty(recFile));

	recLines = readLines(recFile);

	errorCase(getCount(recLines) < 2); // EXEC の場合、空行も無くいきなり終わる。
//	errorCase(getCount(recLines) < 4);

	cwList = GetColumnWidthList(getLine(recLines, 1));

	foreach (recLines, recLine, rowidx)
	{
		if (rowidx == 1) // ? ボーダーラインの行
			continue;

		if (*recLine == '\0') // ? 空行
		{
			// EXEC の場合、空行も無くいきなり終わる。
			errorCase(!lineExp("(<>)", getLine(recLines, rowidx + 1))); // "(n 行処理されました)"
			break;
		}
		line2JLine(recLine, 1, 0, 0, 1);
		row = SplitRow(recLine, cwList);

		foreach (row, column, colidx)
			trimEdge(column, ' ');

		addElement(table, (uint)row);
	}
//	errorCase(!lineExp("(<>)", getLine(recLines, rowidx + 1))); // "(n 行処理されました)" // moved

	releaseDim(recLines, 1);
	releaseAutoList(cwList);

	// check
	{
		uint rowidx;
		uint colcnt;
		uint colidx;

		errorCase(getCount(table) < 1); // ? 行が１つも無い。(カラム名の行は必ずあるはず)

		colcnt = getCount(getList(table, 0));

		errorCase(colcnt < 1); // ? 列が１つも無い。

		for (rowidx = 1; rowidx < getCount(table); rowidx++)
		{
			errorCase(getCount(getList(table, rowidx)) != colcnt); // ? 列数が一定ではない。

			for (colidx = 0; colidx < colcnt; colidx++)
			{
				errorCase(getLine(getList(table, rowidx), colidx) == NULL); // ? cell == NULL
			}
		}
	}
	return table;
}
