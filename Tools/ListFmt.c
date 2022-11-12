/*
	ListFmt.exe (/F 入力ファイル | /LSS | /C 初期値 最大値 増分 Z-PAD)... [/-M | /X] [/-] フォーマット...

		初期値   ... 0 ～ IMAX
		最大値   ... 初期値 ～ IMAX
		増分     ... 1 ～ IMAX
		Z-PAD    ... 0 ～ IMAX

	- - -
	例:

	ListFmt.exe /C 1 9999 1 3 /F ListFmt.c /-M "$DListFmt.c$D の $1 行目: $2"
*/

#include "C:\Factory\Common\all.h"

#define LIST_FILE_MAX 9

#define S_ESCAPE "\1"
#define T_ESCAPE "\2"

static autoList_t *ListFiles;
static autoList_t *WorkFiles;
static int MinLCMode;
static int XMode;
static char *Format;

static autoList_t *Lists;
static autoList_t *CurrLines;

static char *GetLine(void)
{
	char *ret = replaceLine(strx(Format), "$$", S_ESCAPE, 0);

	ret = replaceLine(ret, "$", T_ESCAPE, 0);

	// ----

	ret = replaceLine(ret, T_ESCAPE "A", "&", 1);
	ret = replaceLine(ret, T_ESCAPE "C", "%", 1);
	ret = replaceLine(ret, T_ESCAPE "D", "\"", 1);
	ret = replaceLine(ret, T_ESCAPE "G", ">", 1);
	ret = replaceLine(ret, T_ESCAPE "I", "^", 1);
	ret = replaceLine(ret, T_ESCAPE "L", "<", 1);
	ret = replaceLine(ret, T_ESCAPE "N", "\n", 1);
	ret = replaceLine(ret, T_ESCAPE "P", "|", 1);
	ret = replaceLine(ret, T_ESCAPE "Q", "'", 1);
	ret = replaceLine(ret, T_ESCAPE "S", " ", 1);
	ret = replaceLine(ret, T_ESCAPE "T", "\t", 1);
	ret = replaceLine(ret, T_ESCAPE "Y", "\\", 1);
	ret = replaceLine(ret, T_ESCAPE "1", refLine(CurrLines, 0), 0);
	ret = replaceLine(ret, T_ESCAPE "2", refLine(CurrLines, 1), 0);
	ret = replaceLine(ret, T_ESCAPE "3", refLine(CurrLines, 2), 0);
	ret = replaceLine(ret, T_ESCAPE "4", refLine(CurrLines, 3), 0);
	ret = replaceLine(ret, T_ESCAPE "5", refLine(CurrLines, 4), 0);
	ret = replaceLine(ret, T_ESCAPE "6", refLine(CurrLines, 5), 0);
	ret = replaceLine(ret, T_ESCAPE "7", refLine(CurrLines, 6), 0);
	ret = replaceLine(ret, T_ESCAPE "8", refLine(CurrLines, 7), 0);
	ret = replaceLine(ret, T_ESCAPE "9", refLine(CurrLines, 8), 0);

	// ----

	ret = replaceLine(ret, T_ESCAPE, "$", 0);

	return replaceLine(ret, S_ESCAPE, "$", 0);
}
static void ProcLine(void)
{
	char *line = GetLine();

	cout("%s\n", line);
	memFree(line);
}
static void ListFmt_0(void)
{
	autoList_t *list;
	uint list_index;
	uint index;
	uint maxCount;

	if (MinLCMode)
	{
		maxCount = UINTMAX;

		foreach (Lists, list, list_index)
			m_minim(maxCount, getCount(list));
	}
	else
	{
		maxCount = 0;

		foreach (Lists, list, list_index)
			m_maxim(maxCount, getCount(list));
	}
	for (index = 0; index < maxCount; index++)
	{
		foreach (Lists, list, list_index)
			setElement(CurrLines, list_index, (uint)getLine(list, index % getCount(list)));

		ProcLine();
	}
}
static void LFX_Next(uint list_index)
{
	autoList_t *list;
	char *line;
	uint index;

	if (list_index == getCount(Lists))
	{
		ProcLine();
		return;
	}
	list = getList(Lists, list_index);

	foreach (list, line, index)
	{
		setElement(CurrLines, list_index, (uint)line);
		LFX_Next(list_index + 1);
	}
}
static void ListFmt_XMode(void)
{
	LFX_Next(0);
}
static void ListFmt(void)
{
	autoList_t *list;
	char *file;
	uint index;

	errorCase_m(!getCount(ListFiles), "1つ以上のファイルを指定して下さい。");
	errorCase_m(LIST_FILE_MAX < getCount(ListFiles), "ファイルが多過ぎます。");

	Lists = newList();

	foreach (ListFiles, file, index)
		addElement(Lists, (uint)readLines(file));

	foreach (Lists, list, index)
		errorCase_m(!getCount(list), "空のファイルが指定されました。");

	CurrLines = newList();
	setCount(CurrLines, getCount(Lists));

	if (XMode)
		ListFmt_XMode();
	else
		ListFmt_0();
}
static char *ZeroPad(uint value, uint minlen)
{
	char *ret = xcout("%u", value);

	while (strlen(ret) < minlen)
		ret = insertChar(ret, 0, '0');

	return ret;
}
int main(int argc, char **argv)
{
	ListFiles = newList();
	WorkFiles = newList();

readArgs:
	if (argIs("/F"))
	{
		addElement(ListFiles, (uint)nextArg());
		goto readArgs;
	}
	if (argIs("/LSS"))
	{
		addElement(ListFiles, (uint)FOUNDLISTFILE);
		goto readArgs;
	}
	if (argIs("/C"))
	{
		autoList_t *list = newList();
		uint count;
		uint maxCount;
		uint step;
		uint zpminlen;

		count    = toValue(nextArg());
		maxCount = toValue(nextArg());
		step     = toValue(nextArg());
		zpminlen = toValue(nextArg());

		m_range(count, 0, IMAX);
		m_range(maxCount, count, IMAX);
		m_range(step, 1, IMAX);
		m_range(zpminlen, 0, IMAX);

		for (; count <= maxCount; count += step)
		{
			addElement(list, (uint)ZeroPad(count, zpminlen));
		}

		// make work-file
		{
			char *file = makeTempPath(NULL);

			writeLines(file, list);
			releaseDim(list, 1);

			addElement(ListFiles, (uint)file);
			addElement(WorkFiles, (uint)file);
		}

		goto readArgs;
	}
	if (argIs("/-M"))
	{
		MinLCMode = 1;
		goto readArgs;
	}
	if (argIs("/X"))
	{
		XMode = 1;
		goto readArgs;
	}
	argIs("/-");

	Format = untokenize(allArgs(), " "); // g
	ListFmt();

	// remove work-files
	{
		char *file;
		uint index;

		foreach (WorkFiles, file, index)
			removeFile(file);
	}
}
