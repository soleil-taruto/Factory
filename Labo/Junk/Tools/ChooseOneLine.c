/*
	ChooseOneLine.exe 入力ファイル [/ER 最近の出力のキャッシュファイル] (/L | /R | /M 同じ行を表示し続ける秒数 | /I インデックス)

		/L ... 全行表示する。
		/R ... ランダムに選ばれた１行を表示する。
		/M ... 時間で選ばれた１行を表示する。
		/I ... 指定された１行を表示する。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

static autoList_t *Lines;
static char *RecentlyLinesFile;

static void PrintLine(uint index)
{
	char *line = getLine(Lines, index);

	cout("%s\n", line);

	if (RecentlyLinesFile)
		addLine2File(RecentlyLinesFile, line);
}
static void DoRandom(void)
{
	PrintLine(mt19937_rnd(getCount(Lines)));
}
static void DoModTime(uint timePerLine)
{
	errorCase(!m_isRange(timePerLine, 1, IMAX));

	PrintLine((uint)((time(NULL) / timePerLine) % getCount(Lines)));
}
int main(int argc, char **argv)
{
	mt19937_initCRnd();

	Lines = readResourceLines(nextArg());
	Lines = autoDistinctLines(Lines);

	errorCase(!getCount(Lines));

	if (argIs("/ER"))
	{
		autoList_t *rcntLines;
		char *line;
		uint index;

		RecentlyLinesFile = nextArg();
		createFileIfNotExist(RecentlyLinesFile);

		rcntLines = readLines(RecentlyLinesFile);

		while (getCount(Lines) / 2 < getCount(rcntLines))
			memFree((char *)desertElement(rcntLines, 0));

		writeLines(RecentlyLinesFile, rcntLines);

		foreach (Lines, line, index)
			if (findLine(rcntLines, line) < getCount(rcntLines))
				*line = '\0';

		trimLines(Lines);
		errorCase(!getCount(Lines)); // 2bs 空にならないはず。

		releaseDim(rcntLines, 1);
	}

	if (argIs("/L"))
	{
		char *line;
		uint index;

		foreach (Lines, line, index)
			cout("%s\n", line);

		return;
	}
	if (argIs("/R"))
	{
		DoRandom();
		return;
	}
	if (argIs("/M"))
	{
		DoModTime(toValue(nextArg()));
		return;
	}
	if (argIs("/I"))
	{
		uint index = toValue(nextArg());

		PrintLine(index);
		return;
	}
	error_m("不明なコマンド引数");
}
