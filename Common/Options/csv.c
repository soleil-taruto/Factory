#include "csv.h"

/*
	TSV のときは '\t' にしてね。
*/
int CSVCellDelimiter = ',';

/*
	開放は、releaseDim(readCSVFile("a.csv"), 2); こんな風に、
*/
autoList_t *readCSVFile(char *file)
{
	FILE *fp = fileOpen(file, "rt");
	int chr;
	autoList_t *table = newList();
	autoList_t *row = newList();

	while ((chr = readChar(fp)) != EOF)
	{
		autoBlock_t *cellBuff = newBlock();

		if (chr == '"')
		{
			for (; ; )
			{
				chr = readChar(fp);

				if (chr == EOF) // セルの途中でファイルが終了した。-> セルとファイルの終端と見なす。
					break;

				if (chr == '"')
				{
					chr = readChar(fp);

					if (chr != '"')
					{
//						errorCase(chr != CSVCellDelimiter && chr != '\n' && chr != EOF);
						break;
					}
				}
				addByte(cellBuff, chr);
			}
		}
		else
		{
			for (; ; )
			{
				if (chr == CSVCellDelimiter || chr == '\n')
					break;

				addByte(cellBuff, chr);
				chr = readChar(fp);

				if (chr == EOF)
					break;
			}
		}
		addElement(row, (uint)unbindBlock2Line(cellBuff));

		if (chr == '\n')
		{
			addElement(table, (uint)row);
			row = newList();
		}
	}
	if (getCount(row))
		addElement(table, (uint)row);
	else
		releaseDim(row, 1);

	fileClose(fp);
	return table;
}
autoList_t *readCSVFileTrim(char *file)
{
	autoList_t *table = readCSVFile(file);

	csvTrim(table);
	return table;
}
autoList_t *readCSVFileTR(char *file)
{
	autoList_t *table = readCSVFileTrim(file);

	csvRect(table);
	return table;
}
void writeCSVFile(char *file, autoList_t *table)
{
	FILE *fp = fileOpen(file, "wt");
	uint rowidx;
	uint colidx;

	for (rowidx = 0; rowidx < getCount(table); rowidx++)
	{
		autoList_t *row = (autoList_t *)getElement(table, rowidx);

		for (colidx = 0; colidx < getCount(row); colidx++)
		{
			char *cell = (char *)getElement(row, colidx);

			if (colidx)
				writeChar(fp, CSVCellDelimiter);

			if (strchr(cell, '"') || strchr(cell, ',') || strchr(cell, '\t') || strchr(cell, '\n')) // ? "" 必要
			{
				char *p;

				writeChar(fp, '"');

				for (p = cell; *p; p++)
				{
					if (*p == '"')
					{
						writeChar(fp, '"');
						writeChar(fp, '"');
					}
					else
					{
						writeChar(fp, *p);
					}
				}
				writeChar(fp, '"');
			}
			else
			{
				writeToken(fp, cell);
			}
		}
		writeChar(fp, '\n');
	}
	fileClose(fp);
}

void csvTrim(autoList_t *table)
{
	autoList_t *row;
	uint rowidx;

	foreach (table, row, rowidx)
	{
		while (getCount(row))
		{
			char *cell = getLine(row, getCount(row) - 1);

			if (*cell) // ? 空セルではない
				break;

			memFree((void *)unaddElement(row));
		}
	}
	while (getCount(table))
	{
		row = (autoList_t *)getElement(table, getCount(table) - 1);

		if (getCount(row)) // ? 空行ではない
			break;

		releaseAutoList((autoList_t *)unaddElement(table));
	}
	if (getCount(table) == 0) // (0 x 0) を回避する。-> (1 x 1) にする。
	{
		addElement(table, (uint)createOneElement((uint)strx("")));
	}
}
void csvRect(autoList_t *table)
{
	autoList_t *row;
	uint rowidx;
	uint maxcolcnt = 0;

	foreach (table, row, rowidx)
	{
		maxcolcnt = m_max(maxcolcnt, getCount(row));
	}
	foreach (table, row, rowidx)
	{
		while (getCount(row) < maxcolcnt)
		{
			addElement(row, (uint)strx(""));
		}
	}
}
void csvSquare(autoList_t *table)
{
	uint rowcnt;
	uint colcnt;

	csvRect(table); // 2bs?

	rowcnt = getCount(table);
	colcnt = getCount((autoList_t *)getElement(table, 0));

	if (rowcnt < colcnt)
	{
		while (rowcnt < colcnt)
		{
			autoList_t *row = newList();
			uint colidx;

			for (colidx = 0; colidx < colcnt; colidx++)
			{
				addElement(row, (uint)strx(""));
			}
			addElement(table, (uint)row);
			rowcnt++;
		}
	}
	else if (colcnt < rowcnt)
	{
		autoList_t *row;
		uint rowidx;

		foreach (table, row, rowidx)
		{
			while (getCount(row) < rowcnt)
			{
				addElement(row, (uint)strx(""));
			}
		}
	}
}
void csvTwist(autoList_t *table)
{
	uint count;
	uint rowidx;
	uint colidx;

	csvTrim(table); // 2bs?
	csvSquare(table); // 2bs?

	count = getCount(table);

	for (rowidx = 0; rowidx < count; rowidx++)
	for (colidx = rowidx + 1; colidx < count; colidx++)
	{
		autoList_t *r = (autoList_t *)getElement(table, rowidx);
		autoList_t *c = (autoList_t *)getElement(table, colidx);
		uint rc;
		uint cr;

		rc = getElement(r, colidx);
		cr = getElement(c, rowidx);

		setElement(r, colidx, cr);
		setElement(c, rowidx, rc);
	}
	csvTrim(table);
	csvRect(table);
}
void csvRot(autoList_t *table, sint degree)
{
	switch ((degree % 360 + 360) % 360)
	{
	case   0:                   break;
	case  90: csvRot90(table);  break;
	case 180: csvRot180(table); break;
	case 270: csvRot270(table); break;

	default:
		error();
	}
}
void csvRot90(autoList_t *table) // 時計回りに90°回転
{
	reverseElements(table);
	csvTwist(table);
}
void csvRot270(autoList_t *table) // 時計回りに270°(反時計回りに90°)回転
{
	csvTwist(table);
	reverseElements(table);
}
void csvRot180(autoList_t *table) // 180°回転
{
	csvRot90(table);
	csvRot90(table);
}

char *LSrchComp(autoList_t *table, uint colidxFind, uint retColidx, char *cellFind, sint (*funcComp)(char *, char *))
{
	autoList_t *row;
	uint rowidx;

	foreach (table, row, rowidx)
	{
		if (!funcComp(cellFind, refLine(row, colidxFind)))
		{
			return refLine(row, retColidx);
		}
	}
	return ""; // not found
}
char *LSrch(autoList_t *table, uint colidxFind, uint retColidx, char *cellFind)
{
	return LSrchComp(table, colidxFind, retColidx, cellFind, strcmp);
}
char *LISrch(autoList_t *table, uint colidxFind, uint retColidx, char *cellFind)
{
	return LSrchComp(table, colidxFind, retColidx, cellFind, _stricmp);
}

// _x
autoList_t *readCSVFile_x(char *file)
{
	autoList_t *out = readCSVFile(file);
	memFree(file);
	return out;
}
autoList_t *readCSVFileTrim_x(char *file)
{
	autoList_t *out = readCSVFileTrim(file);
	memFree(file);
	return out;
}
autoList_t *readCSVFileTR_x(char *file)
{
	autoList_t *out = readCSVFileTR(file);
	memFree(file);
	return out;
}
void writeCSVFile_cx(char *file, autoList_t *table)
{
	writeCSVFile(file, table);
	releaseDim(table, 2);
}
void writeCSVFile_xc(char *file, autoList_t *table)
{
	writeCSVFile(file, table);
	memFree(file);
}
void writeCSVFile_xx(char *file, autoList_t *table)
{
	writeCSVFile_cx(file, table);
	memFree(file);
}
