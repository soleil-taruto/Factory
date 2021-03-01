/*
	RemoveSync.exe 名前
*/

#include "C:\Factory\Common\all.h"

static char *RootDir = "C:\\Dev";
static char *S_TargetExts = "c.h.cpp.cs";
static char *RangeName;

static autoList_t *TargetExts;

static void SZTEL_ZeroToEmptyLine(autoList_t *lines)
{
	char *line;
	uint index;

	foreach(lines, line, index)
		if(!line)
			setElement(lines, index, (uint)strx(""));
}
static void SZTEL_SqZeroToZero(autoList_t *lines)
{
	uint index;

restart:
	for(index = 1; index < getCount(lines); index++)
	{
		if(
			!getElement(lines, index - 1) &&
			!getElement(lines, index - 0)
			)
		{
			desertElement(lines, index);

			goto restart; // HACK: 雑
		}
	}
}
static void SqZeroToEmptyLine(autoList_t *lines)
{
	SZTEL_SqZeroToZero(lines);
	SZTEL_ZeroToEmptyLine(lines);
}
static void RemoveAroundEmptyLines(autoList_t *lines)
{
	char *line;
	uint index;

restart:
	foreach(lines, line, index)
	{
		if(line && !*line) // ? 空行
		if(
			index && !getElement(lines, index - 1) ||
			index + 1 < getCount(lines) && !getElement(lines, index + 1)
			)
		{
			memFree(line);
			setElement(lines, index, 0);

			goto restart; // HACK: 雑
		}
	}
}
static uint FindEnd(autoList_t *lines, uint index, uint startTabIndent)
{
	for(; index < getCount(lines); index++)
	{
		char *line = getLine(lines, index);
		char *tLine;
		uint tabIndent = 0;

		tLine = strx(line);

		while(*tLine == '\t')
		{
			eraseChar(tLine);
			tabIndent++;
		}
		if(!strcmp(tLine, "// < sync"))
		{
			errorCase(tabIndent != startTabIndent); // ? 開始・終了インデント幅が違う。
			goto endFunc;
		}
		memFree(tLine);
	}
	error(); // 見つからない。

endFunc:
	return index;
}
static void RemoveSync_File(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;
	int modified = 0;

//	cout("%s\n", file);

	foreach(lines, line, index)
	{
		char *tLine = strx(line);
		uint tabIndent = 0;
		char *targTLine = xcout("// sync > @ %s", RangeName);

		while(*tLine == '\t')
		{
			eraseChar(tLine);
			tabIndent++;
		}
		if(!strcmp(tLine, targTLine))
		{
			uint end = FindEnd(lines, index + 1, tabIndent);

			memFree(line);
			setElement(lines, index, 0);

			memFree((char *)getElement(lines, end));
			setElement(lines, end, 0);

			index = end;
			modified = 1;
		}
		memFree(tLine);
		memFree(targTLine);
	}
	if(modified)
	{
		cout("> %s\n", file);

		RemoveAroundEmptyLines(lines);

		SqZeroToEmptyLine(lines);

		semiRemovePath(file);
		writeLines(file, lines);
	}
	releaseDim(lines, 1);
}
static void RemoveSync(void)
{
	autoList_t *files = lssFiles(RootDir);
	char *file;
	uint index;

	foreach(files, file, index)
	if(findLineCase(TargetExts, getExt(file), 1) < getCount(TargetExts))
	{
		RemoveSync_File(file);
	}
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	// -- init gvar --

	TargetExts = tokenize(S_TargetExts, '.');

	// --

	RangeName = nextArg();

	errorCase(m_isEmpty(RangeName));

	RemoveSync();
}
