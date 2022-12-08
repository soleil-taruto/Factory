#include "all.h"

char *getLine(autoList_t *lines, uint index)
{
	char *line = (char *)getElement(lines, index);

	errorCase(!line);
	return line;
}
char *refLine(autoList_t *lines, uint index)
{
	char *line = (char *)refElement(lines, index);

	if (!line)
		line = "";

	return line;
}
char *zSetLine(autoList_t *lines, uint index, char *line)
{
	return (char *)zSetElement(lines, index, (uint)line);
}

autoList_t *copyLines(autoList_t *lines)
{
	char *line;
	uint index;

	lines = copyAutoList(lines);

	foreach (lines, line, index)
	{
		setElement(lines, index, (uint)strx(getLine(lines, index)));
	}
	return lines;
}
void addLines(autoList_t *lines, autoList_t *subLines)
{
	char *line;
	uint index;

	foreach (subLines, line, index)
	{
		addElement(lines, (uint)strx(line));
	}
}

uint findLineComp(autoList_t *lines, char *lineFind, sint (*funcComp)(char *, char *))
{
	return findElement(lines, (uint)lineFind, (sint (*)(uint, uint))funcComp);
}
uint findLine(autoList_t *lines, char *lineFind)
{
	return findLineComp(lines, lineFind, strcmp);
}
uint findLineCase(autoList_t *lines, char *lineFind, uint ignoreCase)
{
	return findLineComp(lines, lineFind, ignoreCase ? _stricmp : strcmp);
}
uint findJLineICase(autoList_t *lines, char *lineFind)
{
	return findLineComp(lines, lineFind, mbs_stricmp);
}

autoList_t *tokenizeMinMax(char *line, int delimiter, uint nummin, uint nummax, char *def_token) // nummax: 0 == INFINITE
{
	autoList_t *tokens = newList();
	autoBlock_t *token = newBlock();
	char *p;

	errorCase(!nummin); // reserved

	for (p = line; *p && (!nummax || getCount(tokens) < nummax - 1); p++)
	{
		if (*p == delimiter)
		{
			addByte(token, '\0');
			addElement(tokens, (uint)strx(directGetBuffer(token)));
			setSize(token, 0);
		}
		else
		{
			addByte(token, *p);
		}
	}
	if (*p)
		ab_addLine(token, p);

	addByte(token, '\0');
	addElement(tokens, (uint)unbindBlock(token));

	while (getCount(tokens) < nummin)
	{
		addElement(tokens, (uint)strx(def_token));
	}
	fixElements(tokens);
	return tokens;
}
autoList_t *tokenize(char *line, int delimiter)
{
	return tokenizeMinMax(line, delimiter, 1, 0, NULL); // nummin == 1 なら def_token を使うことはないので NULL で良い。
}
autoList_t *tokenizeDs(char *line, char *delims) // delims が MBC-1,2 を含まないとき mbs_ として使用できる。
{
	errorCase(m_isEmpty(delims));

	unizChar(line, delims + 1, *delims);
	return tokenize(line, *delims);
}
char *untokenize(autoList_t *tokens, char *separator)
{
	autoBlock_t *buffLine = newBlock();
	char *token;
	uint index;
	autoBlock_t tmpBlock;

	foreach (tokens, token, index)
	{
		if (index)
			addBytes(buffLine, gndBlockLineVar(separator, tmpBlock));

		addBytes(buffLine, gndBlockLineVar(token, tmpBlock));
	}
	return unbindBlock2Line(buffLine);
}

void sortJLinesICase(autoList_t *lines) // ls系の戻り値のソート
{
	rapidSort(lines, (sint (*)(uint, uint))mbs_stricmp);
}
void trimLines(autoList_t *lines)
{
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		if (!*line) // ? line == ""
		{
			memFree(line);
			setElement(lines, index, 0);
		}
	}
	removeZero(lines);
}
void trimEdgeLines(autoList_t *lines)
{
	while (getCount(lines) && !*(char *)getLastElement(lines))
		memFree((char *)unaddElement(lines));

	while (getCount(lines) && !*getLine(lines, 0))
		memFree((char *)desertElement(lines, 0));
}
void emptizeSubLines(autoList_t *lines, uint start, uint count)
{
	uint index;

	errorCase(getCount(lines) < start);
	errorCase(getCount(lines) - start < count);

	for (index = start; index < start + count; index++)
	{
		getLine(lines, index)[0] = '\0';
	}
}
void emptizeFollowLines(autoList_t *lines, uint start)
{
	errorCase(getCount(lines) < start);
	emptizeSubLines(lines, start, getCount(lines) - start);
}
void rmtrimSubLines(autoList_t *lines, uint start, uint count)
{
	emptizeSubLines(lines, start, count);
	trimLines(lines);
}
void rmtrimFollowLines(autoList_t *lines, uint start)
{
	rmtrimSubLines(lines, start, getCount(lines) - start);
}

void shootingStarLines_CSP(autoList_t *lines, uint count, int starChr, int padChr) // count: 0 == UINTMAX
{
	if (!count)
		count = UINTMAX;

	for (; count; count--)
	{
		char *line;
		uint index;
		sint farthest = -1;

		foreach (lines, line, index)
		{
			char *p = strchr(line, starChr);

			if (p)
			{
				uint i = (uint)p - (uint)line;

				m_maxim(farthest, (sint)i);
			}
		}
		if (farthest == -1)
			break;

		foreach (lines, line, index)
		{
			char *p = strchr(line, starChr);

			if (p)
			{
				uint i = (uint)p - (uint)line;
				uint n = (uint)farthest - i;

				line[i] = padChr;
				line = insertLine_x(line, i, repeatChar(padChr, n));

				setElement(lines, index, (uint)line);
			}
		}
	}
}
void shootingStarLines(autoList_t *lines)
{
	shootingStarLines_CSP(lines, 0, '*', ' ');
}
void spacingStarLines_SP(autoList_t *lines, uint lenmin, int starChr, int padChr)
{
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		char *p = strchr(line, starChr);

		if (p)
		{
			uint i = (uint)p - (uint)line;

			line[i] = padChr;

			if (strlen(line) < lenmin)
			{
				uint n = lenmin - strlen(line);

				line = insertLine_x(line, i, repeatChar(padChr, n));

				setElement(lines, index, (uint)line);
			}
		}
	}
}
void spacingStarLines(autoList_t *lines, uint lenmin)
{
	spacingStarLines_SP(lines, lenmin, '*', ' ');
}

// _x
void addLines_x(autoList_t *lines, autoList_t *subLines)
{
#if 1
	addElements(lines, subLines);
	releaseAutoList(subLines);
#else // 同じ
	addLines(lines, subLines);
	releaseDim(subLines, 1);
#endif
}
autoList_t *tokenize_x(char *line, int delimiter)
{
	autoList_t *out = tokenize(line, delimiter);
	memFree(line);
	return out;
}
char *untokenize_cx(autoList_t *tokens, char *separator)
{
	char *out = untokenize(tokens, separator);
	memFree(separator);
	return out;
}
char *untokenize_xc(autoList_t *tokens, char *separator)
{
	char *out = untokenize(tokens, separator);
	releaseDim(tokens, 1);
	return out;
}
char *untokenize_xx(autoList_t *tokens, char *separator)
{
	char *out = untokenize_xc(tokens, separator);
	memFree(separator);
	return out;
}
