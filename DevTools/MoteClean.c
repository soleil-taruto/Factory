/*
	MoteClean.exe [/F] 対象DIR

		/F ... 削除前に確認しない。(強制モード)

	対象DIRとその配下の 1 ～ 999 または 1.* ～ 999.* を削除する。
*/

#include "C:\Factory\Common\all.h"

static int ForceMode;

static void MoteClean(char *dir)
{
	autoList_t *paths = lss(dir);
	char *path;
	uint index;
	autoList_t *killPaths = newList();

	foreach (paths, path, index)
	{
		int killFlag = 0;
		char *chkPtn = strx(getLocal(path));
		uint numb;

		chkPtn = replaceLine(chkPtn, " - コピー.", ".", 1);

		for (numb = 1; numb < 300; numb++)
			chkPtn = replaceLine_xc(chkPtn, xcout(" - コピー (%u).", numb), ".", 1);

		if (
			lineExp("<1,19><0,2,09>", chkPtn) ||
			lineExp("<1,19><0,2,09>.<>", chkPtn) &&
			lineExp("<1,19><0,2,09>.<>.<>", chkPtn) == 0
			)
			killFlag = 1;

		if (lineExp("<>.tmp", chkPtn))
			killFlag = 1;

		if (killFlag)
		{
			cout("* %s\n", path);
			addElement(killPaths, (uint)strx(path));
		}
		memFree(chkPtn);
	}
	releaseDim(paths, 1);

	if (!ForceMode && getCount(killPaths))
	{
		cout("削除？\n");

		if (clearCoWaitKey(60000) == 0x1b) // GitReleaseAll.bat のとき止まらないように、
//		if (clearGetKey() == 0x1b)
			termination(0);

		cout("削除する。\n");
	}
	foreach (killPaths, path, index)
	{
		cout("! %s\n", path);
		recurRemovePathIfExist(path);
	}
	releaseDim(killPaths, 1);
}
int main(int argc, char **argv)
{
	if (argIs("/F"))
	{
		ForceMode = 1;
	}
	MoteClean(hasArgs(1) ? nextArg() : c_dropDir());
}
