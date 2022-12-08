/*
	Count.exe [/-S] [/LSS | 入力ディレクトリ | 入力ファイル]

	----

	HACK: .\linenum.c と被ってる。
*/

#include "C:\Factory\Common\all.h"

static uint CountLineFromStdin(void)
{
	int entFlg = 0;
	uint count = 0;

	for (; ; )
	{
		int chr = readChar(stdin);

		if (chr == '\n')
		{
			entFlg = 0;
			count++;
		}
		else if (chr == EOF)
		{
			if (entFlg)
				count++;

			break;
		}
		else
		{
			entFlg = 1;
		}
	}
	return count;
}
static uint GetCountLines_x(autoList_t *lines)
{
	uint count = getCount(lines);

	releaseDim(lines, 1);
	return count;
}
static void DispCount(uint count)
{
	cout("%u\n", count);
}
int main(int argc, char **argv)
{
	int ignoreSubDir = 0;

	if (argIs("/-S"))
	{
		ignoreSubDir = 1;
	}

	if (argIs("/LSS"))
	{
		DispCount(GetCountLines_x(readLines(FOUNDLISTFILE)));
		return;
	}
	if (hasArgs(1))
	{
		char *path = nextArg();
		autoList_t *lines;

		if (existDir(path))
			lines = (ignoreSubDir ? ls : lss)(path);
		else
			lines = readLines(path);

		DispCount(GetCountLines_x(lines));
		return;
	}
	DispCount(CountLineFromStdin());
}
