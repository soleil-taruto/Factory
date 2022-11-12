/*
	Sabun.exe [/L] [/S] [/-D] [/B] [/-B] [/DL] [/-R] [�����̃f�B���N�g�� �E���̃f�B���N�g�� [�o�͐�f�B���N�g��]]

		/L  ... �����ɂ̂ݑ��݂���t�@�C�����o�͂���B�f�t�H���g�ł͂��Ȃ��B
		/S  ... �����ɑ��݂��A���e�������t�@�C�����o�͂���B�f�t�H���g�ł͂��Ȃ��B
		/-D ... �����ɑ��݂��A���e���قȂ�t�@�C�����o�͂��Ȃ��B�f�t�H���g�ł͂���B
		/B  ... �����ɑ��݂���t�@�C�����o�͂���B  �f�t�H���g�ł͓��e���قȂ�t�@�C���̂ݏo�͂���B
		/-B ... �����ɑ��݂���t�@�C�����o�͂��Ȃ��B�f�t�H���g�ł͓��e���قȂ�t�@�C���̂ݏo�͂���B
		/DL ... �����ɑ��݂��A���e���قȂ�t�@�C�����o�͂���ꍇ�A�����̃t�@�C�����o�͂���B�f�t�H���g�ł͉E���̃t�@�C�����o�͂���B
		/-R ... �E���ɂ̂ݑ��݂���t�@�C�����o�͂��Ȃ��B�f�t�H���g�ł͂���B
*/

#include "C:\Factory\Common\all.h"

static int OutputLeftOnly  = 0; // ���ɂ̂ݑ��݂���B
static int OutputSame      = 0; // �����ɑ��݂��A���e���������́B
static int OutputDiff      = 1; // �����ɑ��݂��A���e���قȂ���́B
static int OutputDiffLeft  = 0; // �����ɑ��݂��A���e���قȂ�Ƃ������o�͂���B(0 == �E���o�͂���)
static int OutputRightOnly = 1; // �E�ɂ̂ݑ��݂���B

static void DoOutput(char *outDir, char *srcDir, char *file)
{
	char *srcFile = combine(srcDir, file);
	char *outFile = combine(outDir, file);

	cout("< %s\n", srcFile);
	cout("> %s\n", outFile);

	errorCase(!existFile(srcFile));
	errorCase( existPath(outFile));

	createPath(outFile, 'X');
	copyFile(srcFile, outFile);

	memFree(srcFile);
	memFree(outFile);

	cout("COPY_OK\n");
}
static void Sabun(char *lDir, char *rDir, char *outDir)
{
	autoList_t *lFiles;
	autoList_t *rFiles;
	autoList_t *bothFiles;
	autoList_t *lOnlyFiles;
	autoList_t *rOnlyFiles;
	char *file;
	uint index;

	lDir   = makeFullPath(lDir);
	rDir   = makeFullPath(rDir);
	outDir = makeFullPath(outDir);

	cout("L.< %s\n", lDir);
	cout("R.< %s\n", rDir);
	cout("O.> %s\n", outDir);
	cout("LO: %d\n", OutputLeftOnly);
	cout("BS: %d\n", OutputSame);
	cout("BD: %d (%c)\n", OutputDiff, OutputDiffLeft ? 'L' : 'R');
	cout("RO: %d\n", OutputRightOnly);

	createPath(outDir, 'd');

	errorCase(!existDir(lDir));
	errorCase(!existDir(rDir));
	errorCase(!existDir(outDir)); // 2bs

	if (lsCount(outDir)) // ? ��ł͂Ȃ��B
	{
#if 1
		error_m("�o�͐�f�B���N�g������ł͂���܂���B"); // ����ϊ�Ȃ����A�A
#else
		errorCase_m(isAbsRootDir(outDir), "�o�͐悪���[�g�f�B���N�g���̏ꍇ�A��̃h���C�u�łȂ���΂Ȃ�܂���B"); // C:\ �Ƃ��S�폜�����烄�o���̂ŁA�A

		cout("+---------------------------------------+\n");
		cout("| �o�͐�f�B���N�g���͋�ł͂���܂���B|\n");
		cout("+---------------------------------------+\n");
		cout("��ɂ���H\n");

		if (getKey() == 0x1b)
			termination(0);

		cout("��ɂ��܂��B\n");

		recurClearDir(outDir);
#endif
	}

	LOGPOS();
	lFiles = lssFiles(lDir);
	LOGPOS();
	rFiles = lssFiles(rDir);
	LOGPOS();

	changeRoots(lFiles, lDir, NULL);
	LOGPOS();
	changeRoots(rFiles, rDir, NULL);
	LOGPOS();

	sortJLinesICase(lFiles);
	LOGPOS();
	sortJLinesICase(rFiles);
	LOGPOS();

	bothFiles  = newList();
	lOnlyFiles = newList();
	rOnlyFiles = newList();

	mergeLines2ICase(lFiles, rFiles, lOnlyFiles, bothFiles, rOnlyFiles);
	LOGPOS();

	if (OutputLeftOnly)
		foreach (lOnlyFiles, file, index)
			DoOutput(outDir, lDir, file);

	LOGPOS();

	if (OutputSame || OutputDiff)
	{
		foreach (bothFiles, file, index)
		{
			if (OutputSame && OutputDiff)
			{
				DoOutput(outDir, OutputDiffLeft ? lDir : rDir, file);
			}
			else
			{
				char *lFile = combine(lDir, file);
				char *rFile = combine(rDir, file);

				cout("C %s\n", file);

				errorCase(!existFile(lFile)); // 2bs?
				errorCase(!existFile(rFile)); // 2bs?

				if (isSameFile(lFile, rFile)) // ? �������e
				{
					if (OutputSame)
						DoOutput(outDir, lDir, file);
				}
				else // ? �قȂ���e
				{
					if (OutputDiff)
						DoOutput(outDir, OutputDiffLeft ? lDir : rDir, file);
				}
				memFree(lFile);
				memFree(rFile);
			}
		}
	}
	LOGPOS();

	if (OutputRightOnly)
		foreach (rOnlyFiles, file, index)
			DoOutput(outDir, rDir, file);

	LOGPOS();

	memFree(lDir);
	memFree(rDir);
	memFree(outDir);
	releaseAutoList(bothFiles);
	releaseAutoList(lOnlyFiles);
	releaseAutoList(rOnlyFiles);

	cout("done!\n");
}
int main(int argc, char **argv)
{
readArgs:
	// basic opt

	if (argIs("/L"))
	{
		OutputLeftOnly = 1;
		goto readArgs;
	}
	if (argIs("/S"))
	{
		OutputSame = 1;
		goto readArgs;
	}
	if (argIs("/-D"))
	{
		OutputDiff = 0;
		goto readArgs;
	}
	if (argIs("/DL"))
	{
		OutputDiffLeft = 1;
		goto readArgs;
	}
	if (argIs("/-R"))
	{
		OutputRightOnly = 0;
		goto readArgs;
	}

	// ext opt

	if (argIs("/B"))
	{
		OutputSame = 1;
		OutputDiff = 1;
		goto readArgs;
	}
	if (argIs("/-B"))
	{
		OutputSame = 0;
		OutputDiff = 0;
		goto readArgs;
	}

	// - - -

	if (hasArgs(3))
	{
		char *lDir;
		char *rDir;
		char *outDir;

		lDir = nextArg();
		rDir = nextArg();
		outDir = nextArg();

		Sabun(lDir, rDir, outDir);
		return;
	}
	if (hasArgs(2))
	{
		char *lDir;
		char *rDir;
		char *outDir;
		char *outDir2;

		lDir = nextArg();
		rDir = nextArg();
		outDir = makeFreeDir();
		outDir2 = combine(outDir, "Sabun.out");

		Sabun(lDir, rDir, outDir2);
		coExecute_x(xcout("START %s", outDir));

		memFree(outDir);
		memFree(outDir2);
		return;
	}

	{
		char *lDir;
		char *rDir;
		char *outDir;
		char *outDir2;

		lDir = dropDir();
		rDir = dropDir();
		outDir = makeFreeDir();
		outDir2 = combine(outDir, "Sabun.out");

		Sabun(lDir, rDir, outDir2);
		coExecute_x(xcout("START %s", outDir));

		memFree(lDir);
		memFree(rDir);
		memFree(outDir);
		memFree(outDir2);
	}
}
