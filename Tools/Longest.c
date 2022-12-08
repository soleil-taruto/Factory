/*
	Longest.exe [/S 最短の下限] [/-S] [/LSS | 入力ディレクトリ | 入力ファイル]

		/-S  ... サブディレクトリを無視

		[/LSS | 入力ディレクトリ | 入力ファイル] を省略した場合 ⇒ D&D

	- - -

	入力ディレクトリの配下のにある「最長・最短のフルパス」を持つファイル・ディレクトリを探し、それぞれの長さを表示します。
	最短の下限を指定しなければ、最短のフルパスは入力ディレクトリそのものになります。
	入力ファイルの場合、最短・最長の行を探し、それぞれの長さを表示します。
*/

#include "C:\Factory\Common\all.h"

static uint ShortestMin = 0;

static void DispLongest_Lines_x(autoList_t *lines)
{
	char *line;
	uint index;
	uint minlen;
	uint maxlen;

	minlen = UINTMAX;
	maxlen = 0;

	foreach (lines, line, index)
	{
		uint clen = strlen(line);

		if (clen < minlen && ShortestMin <= clen)
		{
			minlen = clen;
			cout("< %u:%s\n", minlen, line);
		}
		if (maxlen < clen)
		{
			maxlen = clen;
			cout("> %u:%s\n", maxlen, line);
		}
	}
	releaseDim(lines, 1);

	cout("----\n");
	cout("%u\n", minlen);
	cout("%u\n", maxlen);
}
static void DispLongest(char *path, int ignoreSubDir)
{
	autoList_t *lines;

	if (existDir(path))
	{
		lines = (ignoreSubDir ? ls : lss)(path);
		insertElement(lines, 0, (uint)makeFullPath(path));
	}
	else
	{
		lines = readLines(path);
	}
	DispLongest_Lines_x(lines);
}
static void DispLongest_LSS(void)
{
	DispLongest_Lines_x(readLines(FOUNDLISTFILE));
}
int main(int argc, char **argv)
{
	int ignoreSubDir = 0;

readArgs:
	if (argIs("/S"))
	{
		ShortestMin = toValue(nextArg());
		cout("Shortest-Min: %u\n", ShortestMin);
		goto readArgs;
	}
	if (argIs("/-S"))
	{
		ignoreSubDir = 1;
		goto readArgs;
	}

	if (argIs("/LSS"))
	{
		DispLongest_LSS();
		return;
	}
	if (hasArgs(1))
	{
		DispLongest(nextArg(), ignoreSubDir);
		return;
	}
	for (; ; )
	{
		char *path = dropPath();

		if (!path)
			break;

		DispLongest(path, ignoreSubDir);
		cout("\n");

		memFree(path);
	}
}
