/*
	TIME_WAIT を数える。

		netstat -n | Grape /c TIME_WAIT

	配下の .txt の AAA を含む行を表示する。

		lss .txt*
		TypeAll /LSS | Grape AAA

	カレントディレクトリ配下の \output\ (outputというディレクトリの配下) を含まないパスの一覧を C:\temp\1 にリダイレクトする。

		dir /s /b | Grape /V \output\ > C:\temp\1
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	int ignoreCase = 0;
	int inverseFlag = 0;
	int showFndNumOnly = 0;
	char *file = NULL;
	autoList_t *patterns = newList();
	char *pattern;
	uint index;
	FILE *fp;
	char *line;
	uint fndnum = 0;

readArgs:
	if (argIs("/I")) // Ignore case
	{
		ignoreCase = 1;
		goto readArgs;
	}
	if (argIs("/V")) // inVerse
	{
		inverseFlag = 1;
		goto readArgs;
	}
	if (argIs("/F")) // input File
	{
		file = nextArg();
		goto readArgs;
	}
	if (argIs("/C")) // count
	{
		showFndNumOnly = 1;
		goto readArgs;
	}
	if (hasArgs(1))
	{
		addElement(patterns, (uint)nextArg());
		goto readArgs;
	}
	fp = file ? fileOpen(file, "rt") : stdin;

	while (line = readLine(fp))
	{
		int fnd = 0;

		foreach (patterns, pattern, index)
		{
			if (mbs_strstrCase(line, pattern, ignoreCase))
			{
				fnd = 1;
				goto found;
			}
		}
	found:
		if (inverseFlag)
			fnd = !fnd;

		if (fnd)
		{
			if (!showFndNumOnly)
				cout("%s\n", line);

			fndnum++;
		}
		memFree(line);
	}
	if (file)
		fileClose(fp);

	if (showFndNumOnly)
		cout("%u\n", fndnum);

	return (int)fndnum;
}
