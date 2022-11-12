#include "csv.h"

char *LineToCSVCell(char *line)
{
	line = strx(line);

	if (strchr(line, '\"') || strchr(line, ',') || strchr(line, '\n')) // ? "" •K—v
	{
		line = replaceLine(line, "\"", "\"\"", 0);
		line = insertChar(line, 0, '\"');
		line = addChar(line, '\"');
	}
	return line;
}
char *RowToCSVLine(autoList_t *row)
{
	char *buffer = strx("");
	char *cell;
	uint index;

	foreach (row, cell, index)
	{
		if (index)
			buffer = addChar(buffer, ',');

		buffer = addLine_x(buffer, LineToCSVCell(cell));
	}
	return buffer;
}
void RowToCSVStream(FILE *fp, autoList_t *row)
{
	char *cell;
	uint index;

	foreach (row, cell, index)
	{
		if (index)
			writeChar(fp, ',');

		writeToken_x(fp, LineToCSVCell(cell));
	}
	writeChar(fp, '\n');
}
autoList_t *CSVStreamToRow(FILE *fp)
{
	autoList_t *row;
	int chr = readChar(fp);

	if (chr == EOF)
		return NULL;

	row = newList();

	for (; ; )
	{
		autoBlock_t *cellBuff = newBlock();

		if (chr == '\"')
		{
			for (; ; )
			{
				chr = readChar(fp);

				if (chr == '\"' && (chr = readChar(fp)) != '\"' || chr == EOF)
					break;

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
		addElement(row, (uint)unbindBlock2Line(cellBuff));

		if (chr == '\n' || chr == EOF)
			break;

		chr = readChar(fp);
	}
	return row;
}
void TrimCSVRow(autoList_t *row, uint mincolcnt)
{
	while (getLine(row, getCount(row) - 1)[0] == '\0')
	{
		memFree((void *)unaddElement(row));
	}
	while (getCount(row) < mincolcnt)
	{
		addElement(row, (uint)strx(""));
	}
}

// _x
char *RowToCSVLine_x(autoList_t *row)
{
	char *retval = RowToCSVLine(row);
	releaseDim(row, 1);
	return retval;
}
void RowToCSVStream_x(FILE *fp, autoList_t *row)
{
	RowToCSVStream(fp, row);
	releaseDim(row, 1);
}
