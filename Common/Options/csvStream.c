#include "csvStream.h"

char *readCSVCell(FILE *fp, int *p_termChr)
{
	autoBlock_t *cellBuff = newBlock();
	int chr = readChar(fp);

	if (chr == '\"')
	{
		for (; ; )
		{
			chr = readChar(fp);

			if (chr == EOF)
				break;

			if (chr == '\"')
			{
				chr = readChar(fp);

				if (chr != '\"')
					break;
			}
			addByte(cellBuff, chr);
		}
	}
	else
	{
		for (; ; )
		{
			if (chr == ',' || chr == '\n' || chr == EOF)
				break;

			addByte(cellBuff, chr);
			chr = readChar(fp);
		}
	}
	if (p_termChr)
		*p_termChr = chr;

	return unbindBlock2Line(cellBuff);
}
autoList_t *readCSVRow(FILE *fp)
{
	autoList_t *row = newList();
	int termChr;

	do
	{
		addElement(row, (uint)readCSVCell(fp, &termChr));
	}
	while (termChr != '\n' && termChr != EOF);

	if (termChr == EOF && getCount(row) == 1 && *getLine(row, 0) == '\0')
	{
		releaseDim(row, 1);
		row = NULL;
	}
	return row;
}

void writeCSVCell(FILE *fp, char *cell)
{
	if (strchr(cell, '\"') || strchr(cell, ',') || strchr(cell, '\n'))
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
void writeCSVRow(FILE *fp, autoList_t *row)
{
	char *cell;
	uint index;

	foreach (row, cell, index)
	{
		if (index)
			writeChar(fp, ',');

		writeCSVCell(fp, cell);
	}
	writeChar(fp, '\n');
}

// _x
void writeCSVCell_x(FILE *fp, char *cell)
{
	writeCSVCell(fp, cell);
	memFree(cell);
}
void writeCSVRow_x(FILE *fp, autoList_t *row)
{
	writeCSVRow(fp, row);
	releaseDim(row, 1);
}
