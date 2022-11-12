/*
	FindCodeLines.exe [/D ROOT-DIR]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"

static int IsMatchCodeLines(autoList_t *lines, uint line_index, autoList_t *codeLines)
{
	char *line     = getLine(lines, line_index);
	char *codeLine = getLine(codeLines, 0);
	uint lineLen;
	uint codeLineLen;
	uint indent;
	uint index;
	uint i;

	lineLen     = strlen(line);
	codeLineLen = strlen(codeLine);

	if (lineLen < codeLineLen)
		return 0;

	indent = lineLen - codeLineLen;

	for (index = 0; index < getCount(codeLines); index++)
	{
		line     = getLine(lines, line_index + index);
		codeLine = getLine(codeLines, index);

		if (*codeLine)
		{
			if (strlen(line) != indent + strlen(codeLine))
				return 0;

			for (i = 0; i < indent; i++)
				if (line[i] != '\t')
					return 0;

			if (strcmp(line + indent, codeLine))
				return 0;
		}
		else
		{
			if (*line)
				return 0;
		}
	}
	return 1;
}
int main(int argc, char **argv)
{
	char *rootDir = "C:\\Dev";
	autoList_t *codeLines;
	autoList_t *codeLines_utf8;

	if (argIs("/D"))
	{
		rootDir = nextArg();
	}
	errorCase_m(hasArgs(1), "不明なコマンド引数");

	codeLines = inputLines();
	codeLines_utf8 = newList();

	while (getCount(codeLines) && *(char *)getLastElement(codeLines) == '\0')
	{
		memFree((char *)unaddElement(codeLines));
	}
	errorCase(!getCount(codeLines));

	{
		char *line;
		uint index;

		foreach (codeLines, line, index)
		{
			{
				char *rFile = makeTempPath(NULL);
				char *wFile = makeTempPath(NULL);

				writeOneLineNoRet_b(rFile, line);
				SJISToUTF8File(rFile, wFile);
				line = readText_b(wFile);

				removeFile_x(rFile);
				removeFile_x(wFile);
			}

			addElement(codeLines_utf8, (uint)line);
		}
	}

	{
		autoList_t *files = lssFiles(rootDir);
		char *file;
		uint index;

		foreach (files, file, index)
		{
			char *ext = getExt(file);

			if (
				!_stricmp(ext, "h") ||
				!_stricmp(ext, "c") ||
				!_stricmp(ext, "cs") ||
				!_stricmp(ext, "cpp")
				)
			{
				autoList_t *lines = readLines(file);
				uint line_index;

				for (line_index = 0; line_index + getCount(codeLines) <= getCount(lines); line_index++)
				{
					if (
						IsMatchCodeLines(lines, line_index, codeLines) ||
						IsMatchCodeLines(lines, line_index, codeLines_utf8)
						)
					{
						cout("発見 ⇒ %s (%u)\n", file, line_index);
					}
				}
				releaseDim(lines, 1);
			}
		}
		releaseDim(files, 1);
	}

	releaseDim(codeLines, 1);
}
