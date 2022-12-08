/*
	���s���܂ރ��R�[�h��z�肵�Ȃ��B
	sqlcmd ���s���� -k2 ��t���邱�ƁI

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

		errorCase(getCount(cwList) < 1); // ? �񂪂P�������B

		for (colidx = 0; colidx < getCount(cwList); colidx++)
		{
			errorCase(getElement(cwList, colidx) < 1); // ? �񕝂��P�����������B
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

			errorCase(*p++ != ' '); // ��̋�؂肪�󔒂ł͂Ȃ��B
		}
		errorCase(!*p); // �s�̓r���ŏI��

		if (isMbc(p))
		{
			column = addChar(column, *p++);
		}
		column = addChar(column, *p++);
		c--;
	}
	errorCase(*p); // �s�̏I�[�ŏI�����Ȃ��B

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

	errorCase(getCount(recLines) < 2); // EXEC �̏ꍇ�A��s�����������Ȃ�I���B
//	errorCase(getCount(recLines) < 4);

	cwList = GetColumnWidthList(getLine(recLines, 1));

	foreach (recLines, recLine, rowidx)
	{
		if (rowidx == 1) // ? �{�[�_�[���C���̍s
			continue;

		if (*recLine == '\0') // ? ��s
		{
			// EXEC �̏ꍇ�A��s�����������Ȃ�I���B
			errorCase(!lineExp("(<>)", getLine(recLines, rowidx + 1))); // "(n �s��������܂���)"
			break;
		}
		line2JLine(recLine, 1, 0, 0, 1);
		row = SplitRow(recLine, cwList);

		foreach (row, column, colidx)
			trimEdge(column, ' ');

		addElement(table, (uint)row);
	}
//	errorCase(!lineExp("(<>)", getLine(recLines, rowidx + 1))); // "(n �s��������܂���)" // moved

	releaseDim(recLines, 1);
	releaseAutoList(cwList);

	// check
	{
		uint rowidx;
		uint colcnt;
		uint colidx;

		errorCase(getCount(table) < 1); // ? �s���P�������B(�J�������̍s�͕K������͂�)

		colcnt = getCount(getList(table, 0));

		errorCase(colcnt < 1); // ? �񂪂P�������B

		for (rowidx = 1; rowidx < getCount(table); rowidx++)
		{
			errorCase(getCount(getList(table, rowidx)) != colcnt); // ? �񐔂����ł͂Ȃ��B

			for (colidx = 0; colidx < colcnt; colidx++)
			{
				errorCase(getLine(getList(table, rowidx), colidx) == NULL); // ? cell == NULL
			}
		}
	}
	return table;
}
