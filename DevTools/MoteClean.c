/*
	MoteClean.exe [/F] �Ώ�DIR

		/F ... �폜�O�Ɋm�F���Ȃ��B(�������[�h)

	�Ώ�DIR�Ƃ��̔z���� 1 �` 999 �܂��� 1.* �` 999.* ���폜����B
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

		chkPtn = replaceLine(chkPtn, " - �R�s�[.", ".", 1);

		for (numb = 1; numb < 300; numb++)
			chkPtn = replaceLine_xc(chkPtn, xcout(" - �R�s�[ (%u).", numb), ".", 1);

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
		cout("�폜�H\n");

		if (clearCoWaitKey(60000) == 0x1b) // GitReleaseAll.bat �̂Ƃ��~�܂�Ȃ��悤�ɁA
//		if (clearGetKey() == 0x1b)
			termination(0);

		cout("�폜����B\n");
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
