/*
	�w��f�B���N�g���z���� .rum �f�B���N�g����S�ă`�F�b�N���܂��B
	�f�t�H���g�ł� CheckRum.exe �����s���Ȃ��B

	CheckRumInDir.exe [/S] [/CR [/-S]] ROOT-DIR

		/S  ... �T�u�f�B���N�g������������B
		/CR ... �f�B���N�g������ CheckRum.exe �����s����B
		/-S ... files.txt, tree.txt �́i���g�́j�p�X�� isFairRelPath() ���Ȃ��B
*/

#include "C:\Factory\Common\all.h"

static int IntoSubDir;
static int DoCheckRum;
static int NoStrict;

static void CollectRumDirs(char *rootDir, autoList_t *dest)
{
	autoList_t *dirs = lsDirs(rootDir);
	char *dir;
	uint index;

	foreach (dirs, dir, index)
	{
		if (!_stricmp("rum", getExt(dir)))
		{
			addElement(dest, (uint)strx(dir));
		}
		else
		{
			if (IntoSubDir)
				CollectRumDirs(dir, dest);
		}
	}
	releaseDim(dirs, 1);
}
static void CheckRum(char *dir)
{
	char *mastDir = changeExt(dir, "");

	cout("%s\n", dir);

	errorCase_m(!existDir(mastDir), "�Ǘ����Ă��܂��B");

	if (DoCheckRum)
	{
		cout("CheckRum.exe �����s���܂��B\n");
		execute_x(xcout("C:\\Factory\\Tools\\CheckRum.exe %s\"%s\"", NoStrict ? "/-S " : "", dir));
		cout("cr_done\n");
	}
	memFree(mastDir);
}
static void CheckRumInDir(char *rootDir)
{
	autoList_t *dirs = newList();
	char *dir;
	uint index;

	rootDir = makeFullPath(rootDir);
	errorCase(!existDir(rootDir));
	CollectRumDirs(rootDir, dirs);

	foreach (dirs, dir, index)
		CheckRum(dir);

	memFree(rootDir);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/S"))
	{
		IntoSubDir = 1;
		goto readArgs;
	}
	if (argIs("/CR"))
	{
		DoCheckRum = 1;
		goto readArgs;
	}
	if (argIs("/-S"))
	{
		NoStrict = 1;
		goto readArgs;
	}

	if (hasArgs(1))
	{
		CheckRumInDir(nextArg());
	}
	else
	{
		char *rootDir = dropDir();

		CheckRumInDir(rootDir);
		memFree(rootDir);
	}

	cout("done\n");
}
