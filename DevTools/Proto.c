/*
	Proto.exe [TARGET-FILE]
*/

#include "C:\Factory\Common\all.h"

static autoList_t *MakeProtoList(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;
	autoList_t *protos = newList();

	foreach (lines, line, index)
	{
		if (line[0] &&
			line[0] != '\t' &&
			line[0] != ' ' &&
			line[0] != '#' &&
			line[0] != '*' &&
			line[0] != '/' &&
			line[0] != '{' &&
			line[0] != '}' &&
			!strstr(line, "static")
			)
		{
			if (strchr(line, '=') || strchr(line, ';')) // ? •Ï”
			{
				strchrEnd(line, '=')[0] = '\0';
				strchrEnd(line, ';')[0] = '\0';

				line = insertLine(line, 0, "extern ");
			}
			strchrEnd(line, '/')[0] = '\0'; // ƒRƒƒ“ƒgíœ

			line = addChar(line, ';');
			trim(line, ' ');

			setElement(lines, index, (uint)line);
			addElement(protos, (uint)strx(line));
		}
	}
	releaseDim(lines, 1);
	return protos;
}

int main(int argc, char **argv)
{
	char *file = hasArgs(1) ? strx(nextArg()) : dropFile();
	autoList_t *list;

	list = MakeProtoList(file);
	viewTextLines(list);

	memFree(file);
	releaseDim(list, 1);
}
