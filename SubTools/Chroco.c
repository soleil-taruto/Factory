/*
	Chroco.exe /R �R�s�[�� /D �R�s�[�� /P �p�X���X�g�t�@�C�� [/-C] [/-N]

	�R�s�[��
		���[�J����ɑ��݂���f�B���N�g���ւ̑��΃p�X�܂��͐�΃p�X
		���΃p�X�̏ꍇ�A���s���̃J�����g�f�B���N�g������Ƃ���B
	�R�s�[��
		���[�J����ɑ��݂��閔�͍쐬�\�ȃf�B���N�g���ւ̑��΃p�X�܂��͐�΃p�X
		���΃p�X�̏ꍇ�A���s���̃J�����g�f�B���N�g������Ƃ���B
	�p�X���X�g�t�@�C��
		�Ώۃp�X�̃��X�g(�p�X���X�g)���L�q�����t�@�C��
		�����R�[�h CP932 ���s�R�[�h CR-LF or LF �̃e�L�X�g�t�@�C���ł��邱�ƁB
		�Ώۃp�X�����s��؂�ŋL�q����B
		�Ώۃp�X�O��ɕs�v�ȋ󔒂����Ă͂Ȃ�Ȃ��B
		��s�����Ă͂Ȃ�Ȃ��B�ŏI�s�̏I�[�ɂ͉��s���L���Ă������Ă��ǂ��B
		�P���ȏ゠�邱�ƁB
	�Ώۃp�X
		���[�J����ɑ��݂���f�B���N�g���܂��̓t�@�C���ւ̑��΃p�X�܂��͐�΃p�X
		���΃p�X�̏ꍇ�A�R�s�[������Ƃ���B
		�R�s�[���̔z���̃p�X�łȂ���΂Ȃ�Ȃ��B�܂��A�R�s�[�����g���w�肵�Ă͂Ȃ�Ȃ��B

	/-C
		�R�s�[�O�ɃR�s�[����N���A���Ȃ��B�f�t�H���g�ł̓N���A����B
	/-N
		�p�X�̑啶���E�������̓p�X���X�g�ɍ��킹��B�f�t�H���g�ł̓R�s�[���̃p�X�ɍ��킹��B
*/

#include "C:\Factory\Common\all.h"

static void ClearDir(char *dir)
{
	autoList_t *paths;
	char *path;
	uint index;

	cout("rmdir: %s\n", dir);

	paths = lss(dir);
	reverseElements(paths);

	foreach (paths, path, index)
	{
		cout("rm: %s\n", path);
		( existDir(path) ? removeDir : removeFile )(path);
	}
	releaseDim(paths, 1);

	cout("rmdir done.\n");

	removeDir(dir);
	createDir(dir);

	cout("re-mkdir ok.\n");
}
static char *NormalizePathCase(char *path, char *dir)
{
	autoList_t *ptkns;
	char *ptkn;
	uint index;

	escapeYen(path);
	ptkns = tokenize(path, '/');
	restoreYen(path);

	addCwd(dir);
	foreach (ptkns, ptkn, index)
	{
		char *newPTkn;

		updateFindData(ptkn);
		newPTkn = strx(lastFindData.name);

		memFree(ptkn);
		setElement(ptkns, index, (uint)newPTkn);

		if (index < getCount(ptkns) - 1)
		{
			changeCwd(newPTkn);
		}
	}
	unaddCwd();

	path = untokenize(ptkns, "\\");
	releaseDim(ptkns, 1);
	return path;
}

static char *RootDir;
static char *DestDir;
static char *PathListFile;
static int NoClearDestDir;
static int NoNormalizePathCase;
static autoList_t *PathList;

static void ExecCopy(char *path, char *dir1, char *dir2) // (dir1, dir2) == absDir
{
	autoList_t *ptkns;
	char *ptkn;
	uint index;
	char *path1;
	char *path2;

	cout("# %s\n", path);
	cout("< %s\n", dir1);
	cout("> %s\n", dir2);

	escapeYen(path);
	ptkns = tokenize(path, '/');
	restoreYen(path);

	memFree((void *)unaddElement(ptkns));

	addCwd(dir2);
	foreach (ptkns, ptkn, index)
	{
		mkdirEx(ptkn);
		changeCwd(ptkn);
	}
	unaddCwd();
	releaseDim(ptkns, 1);

	path1 = combine(dir1, path);
	path2 = combine(dir2, path);

	if (existFile(path1))
		copyFile(path1, path2);
	else
		mkdirEx(path2);

	memFree(path1);
	memFree(path2);
}
static void ProcCopy(void)
{
	char *path;
	uint index;

	RootDir = makeFullPath(RootDir);
	DestDir = makeFullPath(DestDir);
	PathList = readLines(PathListFile);

	mkdirEx(DestDir);

	errorCase_m(!existDir(RootDir), "�R�s�[���̃f�B���N�g���ɃA�N�Z�X�ł��܂���B");
	errorCase_m(!existDir(DestDir), "�R�s�[��̃f�B���N�g���ɃA�N�Z�X�ł��܂���B");
	errorCase_m(!getCount(PathList), "�p�X���X�g����ł��B");

	cout("[R] %s\n", RootDir);
	cout("[D] %s\n", DestDir);

	foreach (PathList, path, index)
	{
		addCwd(RootDir);
		path = makeFullPath_x(path);
		unaddCwd();

		cout("[%u] %s\n", index + 1, path);

		errorCase_m(!existPath(path), xcout("�R�s�[���悤�Ƃ����p�X�ɃA�N�Z�X�ł��܂���B\n%s", path));
		changeRoot(path, RootDir, NULL);
		errorCase(!*path);

		setElement(PathList, index, (uint)path);
	}
	if (!NoNormalizePathCase)
	{
		foreach (PathList, path, index)
		{
			char *newPath = NormalizePathCase(path, RootDir);

			if (strcmp(path, newPath))
			{
				cout("< %s\n", path);
				cout("> %s\n", newPath);
			}
			memFree(path);
			setElement(PathList, index, (uint)newPath);
		}
	}
	if (!NoClearDestDir)
	{
		ClearDir(DestDir);
	}
	foreach (PathList, path, index)
	{
		ExecCopy(path, RootDir, DestDir);
	}

	memFree(RootDir);
	memFree(DestDir);
	releaseDim(PathList, 1);

	cout("\\e\n");
}

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/R")) // Root dir
	{
		RootDir = nextArg();
		goto readArgs;
	}
	if (argIs("/D")) // Destination dir
	{
		DestDir = nextArg();
		goto readArgs;
	}
	if (argIs("/P")) // Path list file
	{
		PathListFile = nextArg();
		goto readArgs;
	}

	if (argIs("/-C")) // no Clear destination dir
	{
		NoClearDestDir = 1;
		goto readArgs;
	}
	if (argIs("/-N")) // no Normalize path case
	{
		NoNormalizePathCase = 1;
		goto readArgs;
	}

	errorCase_m(hasArgs(1), "�s���ȃR�}���h����������܂��B");

	errorCase_m(m_isEmpty(RootDir), "�R�s�[���̃f�B���N�g�����w�肵�ĉ������B");
	errorCase_m(m_isEmpty(DestDir), "�R�s�[��̃f�B���N�g�����w�肵�ĉ������B");
	errorCase_m(m_isEmpty(PathListFile), "�p�X���X�g�t�@�C�����w�肵�ĉ������B");

	ProcCopy();
}
