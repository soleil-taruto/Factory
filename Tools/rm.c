/*
	rm.exe [/F] ...

		/F ... �m�F���s��Ȃ��B

	rm.exe ... /LSS

		�Ō�̃t�@�C�����X�g�̑S�Ẵt�@�C�����폜����B(lss�Ƃ̕��p��z��)

	rm.exe ... /D [FILE-OR-DIR]

		�t�@�C���܂��̓f�B���N�g�����폜����B������Ή������Ȃ��B

	rm.exe ... [DIR]

		�f�B���N�g���̒����̃t�@�C���E�f�B���N�g����I�����č폜����B
*/

#include "C:\Factory\Common\all.h"

static int ForceMode;

static void RemoveEx(autoList_t *paths)
{
	char *path;
	uint index;
	char *destDir;
	autoList_t *restoreCmds = newList();

	if (!ForceMode)
	{
		foreach (paths, path, index)
		{
			cout("* %s\n", path);
		}
		cout("�폜�H\n");

		if (clearGetKey() == 0x1b)
			termination(1);
	}
	destDir = makeFreeDir();

	foreach (paths, path, index)
	{
		char *destPath = combine(destDir, getLocal(path));

		destPath = toCreatablePath(destPath, index);

		cout("< %s\n", path);
		cout("> %s\n", destPath);

		if (existDir(path))
		{
			createDir(destPath);
			moveDir(path, destPath);
			removeDir(path);
		}
		else
		{
			moveFile(path, destPath);
		}
		addElement(restoreCmds, (uint)xcout("MOVE \"%s\" \"%s\"", destPath, c_makeFullPath(path)));
		memFree(destPath);
	}

	if (getCount(restoreCmds))
	{
		char *batch = combine(destDir, "_���X�g�A.bat_");

		batch = toCreatablePath(batch, getCount(paths));
		cout("# %s\n", batch);
		writeLines(batch, restoreCmds);
		memFree(batch);
	}
	else
	{
		removeDir(destDir);
	}
	memFree(destDir);
	releaseDim(restoreCmds, 1);
}
static void RemoveExOne(char *path)
{
	uint geb;
	autoList_t gab;

	if (!existPath(path))
	{
		cout("�w�肳�ꂽ�p�X��������Ȃ��̂ŁA�������܂���B\n");
		return;
	}
	RemoveEx(gndOneElementVar((uint)path, geb, gab));
}
static void SelectRemoveEx(char *dir)
{
	autoList_t *lpaths = ls(dir);

	eraseParents(lpaths);
	lpaths = selectLines_x(lpaths);

	addCwd(dir);
	RemoveEx(lpaths);
	unaddCwd();

	releaseDim(lpaths, 1);
}

int main(int argc, char **argv)
{
	if (argIs("/F"))
	{
		ForceMode = 1;
	}
	if (argIs("/LSS"))
	{
		autoList_t *paths = readLines(FOUNDLISTFILE);

		if (getCount(paths))
		{
			RemoveEx(paths);
		}
		releaseDim(paths, 1);
		return;
	}
	if (argIs("/D"))
	{
		RemoveExOne(hasArgs(1) ? nextArg() : c_dropDirFile());
		return;
	}
	SelectRemoveEx(hasArgs(1) ? nextArg() : c_dropDir());
}
