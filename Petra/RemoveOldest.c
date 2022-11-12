/*
	RemoveOldest.exe 親フォルダ キープ数

	- - -

	親フォルダ直下のキープ数を超えるパスについて「古いパス」から削除する。
	古いパスとは、辞書順で先にくるものとする。
*/

#include "C:\Factory\Common\all.h"

static void RemoveOldest(char *rootDir, uint keepNum)
{
	autoList_t *paths;
	char *path;
	uint index;

	rootDir = makeFullPath(rootDir);

	errorCase(!existDir(rootDir));
	errorCase(!m_isRange(keepNum, 1, IMAX));

	paths = lss(rootDir);
	sortJLinesICase(paths);

	LOGPOS();

	while (keepNum < getCount(paths))
	{
		LOGPOS();
		removePath_x((char *)desertElement(paths, 0));
		LOGPOS();
	}
	LOGPOS();

	releaseDim(paths, 1);
}
int main(int argc, char **argv)
{
	char *rootDir;
	uint keepNum;

	rootDir = nextArg();
	keepNum = toValue(nextArg());

	RemoveOldest(rootDir, keepNum);
}
