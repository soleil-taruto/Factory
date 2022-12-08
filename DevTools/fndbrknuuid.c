/*
	fndbrknuuid.exe [TARGET-DIR]
*/

#include "C:\Factory\Common\all.h"

#define NB_UUID_FMT "99999999-9999-9999-9999-999999999999"

static uint ExitCode = 0;

static void ToFormat(char *p)
{
	for (; *p; p++)
		if (strchr("012345678abcdef", *p))
			*p = '9';
}
static void ToNoJpn(char *p)
{
	while (*p)
	{
		if (isMbc(p))
		{
			*p++ = 'J';
			*p++ = 'J';
		}
		else
		{
			p++;
		}
	}
}

static char *CurrFile;
static uint CurrLineIndex;
static char *CurrLine;

static void FindError(char *message)
{
	cout("----\n");
	cout("【警告】%s\n", message);
	cout("%s\n", CurrFile);
	cout("%u 行目\n", CurrLineIndex + 1);
	cout("%s\n", CurrLine);

	ExitCode = 1;
}
static char *DoCheck(char *p)
{
	char *q;

	p = strchr(p, '{');

	if (!p)
		return NULL;

	p++;
	q = strchr(p, '}');

	if (!q)
	{
		FindError("閉じていません。");
		return p;
	}
	*q = '\0';
	q++;
	ToFormat(p);

	if (strcmp(p, NB_UUID_FMT))
	{
		FindError("ブラケット内が破損しているようです。");
	}
	return q;
}
static int IsUUIDPoi(char *line)
{
	if (lineExp("<>// not_uuid", line)) return 0;

	if (lineExp("<>{<>}<>shared_uuid<>", line)) return 1;

	if (lineExp("<>\"<>{<>}<>\"<>", line))
	{
		if (lineExp("<>\"<>{}<>\"<>", line)) return 0;
		if (lineExp("<>\"<>{ <>}<>\"<>", line)) return 0;
		if (lineExp("<>\"<>{<> }<>\"<>", line)) return 0;
		if (lineExp("<>\"<>{<,10,>}<>\"<>", line)) return 0;
		if (lineExp("<>\"<>{\"<>\"}<>\"<>", line)) return 0;
		if (lineExp("<>\"<>{\\\"<>\"}<>\"<>", line)) return 0;

		if (lineExp("<>\"<>${<>}<>\"<>", line)) return 0; // added @ 2019.2.9

		return 1;
	}
	return 0;
}
static void FindBrokenUUID_File(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		char *nj_line = strx(line);

		ToNoJpn(nj_line);

		if (IsUUIDPoi(nj_line)) // ? uuid らしき行
		{
			char *p = nj_line;

			CurrFile = file;
			CurrLineIndex = index;
			CurrLine = line;

			while (p = DoCheck(p));

			CurrFile = NULL;
			CurrLineIndex = 0;
			CurrLine = NULL;
		}
		memFree(nj_line);
	}
	releaseDim(lines, 1);
}
static int IsTargetFile(char *file)
{
	char *ext;

	if (mbs_stristr(file, "fndbrknuuid"))
		return 0;

	ext = getExt(file);

	return
		!_stricmp(ext, "c") ||
		!_stricmp(ext, "h") ||
		!_stricmp(ext, "cs") ||
		!_stricmp(ext, "cpp") ||
		!_stricmp(ext, "java");
}
static void FindBrokenUUID(char *dir)
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;

	foreach (files, file, index)
		if (IsTargetFile(file))
			FindBrokenUUID_File(file);

	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	antiSubversion = 1;

	while (hasArgs(1))
	{
		FindBrokenUUID(nextArg());
	}
	termination(ExitCode);
}
