/*
	ds.exe [[/S] 入力ディレクトリ]

		/S ... シンプルモード
*/

#include "C:\Factory\Common\all.h"

static void DirSize(char *dir)
{
	autoList_t *paths = ls(dir);
	char *path;
	uint index;
	uint dircnt;
	uint64 size;
	uint64 totalSize = 0;
	autoList_t *lines = newList();
	char *line;
	char *sSize;
	char *localFile;

	dircnt = lastDirCount;

	foreach (paths, path, index)
	{
		cmdTitle_x(xcout("ds - %s (%u / %u)", path, index, getCount(paths)));

		size = ( index < dircnt ? getDirSize : getFileSize )(path);
		totalSize += size;

		addElement(lines, (uint)xcout("%020I64u%s", size, getLocal(path)));
	}
	cmdTitle("ds");
	rapidSortLines(lines);

	foreach (lines, line, index)
	{
		sSize = strxl(line, 20);
		localFile = strx(line + 20);

		while (sSize[0] == '0' && sSize[1])
			eraseChar(sSize);

		sSize = thousandComma(sSize);

		memFree(line);
		line = xcout("%s*%s", localFile, sSize);
		setElement(lines, index, (uint)line);

		memFree(sSize);
		memFree(localFile);
	}
	spacingStarLines(lines, 79);

	foreach (lines, line, index)
		cout("%s\n", line);

	line = xcout("%I64u", totalSize);
	line = thousandComma(line);

	cout("\n");
	cout("合計サイズ %68s\n", line);
	cout("\n");

	memFree(line);

	releaseDim(paths, 1);
	releaseDim(lines, 1);
}
int main(int argc, char **argv)
{
	if (argIs("/S")) // Simple mode
	{
		char *sSize = xcout("%I64u", getDirSize(nextArg()));

		sSize = thousandComma(sSize);
		cout("%s\n", sSize);
		memFree(sSize);
		return;
	}
	if (hasArgs(1))
	{
		DirSize(nextArg());
		cout("\\e\n");
		return;
	}
	for (; ; )
	{
		char *dir = dropDir();

		DirSize(dir);
		memFree(dir);
	}
}
