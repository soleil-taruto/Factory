/*
	.zip .7z �̓W�J�̂�

	- - -

	7z.exe [/ANO] [/C] [/T] [/OAD] [/P �p�X�t���[�Y] [7z-FILE | ZIP-FILE]

		/ANO ... �����W�J����Ȃ������ꍇ�ł��G���[�ɂ��Ȃ��B
		/C   ... ���̓t�@�C���Ɠ����ꏊ�ɓW�J����B
		/T   ... �s�v�ȏ�ʊK�w����������B
		/OAD ... ���t�@�C�������폜
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Collabo.h"
#include "C:\Factory\Meteor\7z.h"

static int AllowNoOutput;
static int ExtractSameDir;
static int TrimOneDir;
static int OutputAndDelete;
static char *Passphrase;

static char *Get7zExeFile(void) // ret: �󔒂��܂܂Ȃ��p�X�ł��邱�ƁB
{
	static char *file;

	if (!file)
		file = GetCollaboFile(FILE_7Z_EXE);

	return file;
}
static char *GetSwitches(void)
{
	static char *ret;

	memFree(ret);
	ret = strx("");

	if (Passphrase)
	{
		errorCase_m(!lineExp("<1,100,--__09AZaz>", Passphrase), "�p�X�t���[�Y�Ɏg���镶���� - _ 0�`9 A�`Z a�`z �ł��B");

		ret = addLine_x(ret, xcout(" -p%s", Passphrase));
	}
	return ret;
}
static char *Unlittering(char *dir, char *file7z)
{
	if (2 <= lsCount(dir))
	{
		char *wRtDir = makeFreeDir();
		char *wDir;
		char *name = changeExt(getLocal(file7z), "");

		wDir = combine(wRtDir, name);
		createDir(wDir);

		LOGPOS();
		cout("< %s\n", dir);
		cout("> %s\n", wDir);

		moveDir(dir, wDir);

		memFree(dir);
		dir = wRtDir;
		memFree(wDir);
		memFree(name);
	}
	return dir;
}
static void DoTrimOneDir(char *wDir)
{
	char *rDir = strx(wDir);
	int dove = 0;

	while (lsCount(rDir) == 1)
	{
		autoList_t *paths = ls(rDir);
		char *path;

		path = getLine(paths, 0);

		if (!existDir(path))
		{
			releaseDim(paths, 1);
			break;
		}
		memFree(rDir);
		rDir = path;
		releaseAutoList(paths);
		dove = 1;
	}
	if (dove)
	{
		char *midDir = strx(wDir);

		midDir = toCreatablePath(midDir, IMAX);

		cout("< %s\n", rDir);
		cout("> %s\n", wDir);
		cout("M %s\n", midDir);

		createDir(midDir);
		moveDir(rDir, midDir);
		recurRemoveDir(wDir);
		moveFile(midDir, wDir);
	}
	memFree(rDir);
}
static void Post7z(char *dir)
{
	if (!AllowNoOutput)
	{
		/*
			/OAD ���w�肵�ăA�[�J�C�u�ł͂Ȃ��t�@�C�����h���b�v���Ă��܂��ƁA
			�����o�͂���Ȃ���A���t�@�C�����폜����Ă��܂��̂ŁA���̑΍�Ƃ��āB
		*/
		errorCase_m(!lsCount(dir), "�����W�J����܂���ł����B");
	}
}
static void Extract7z(char *file7z)
{
	char *dir;
	char *dir2;

	file7z = makeFullPath(file7z);

	cout("< %s\n", file7z);

	errorCase(!existFile(file7z));
	errorCase(!*getExt(file7z)); // ? �g���q�i�V

	if (ExtractSameDir)
	{
		dir = changeExt(file7z, "");
		dir = toCreatableTildaPath(dir, 10000);

		cout("> %s\n", dir);

		createDir(dir);
		addCwd(dir);
		coExecute_x(xcout("%s x%s \"%s\"", Get7zExeFile(), GetSwitches(), file7z));
		unaddCwd();
		Post7z(dir);

		if (TrimOneDir)
			DoTrimOneDir(dir);
	}
	else
	{
		dir = makeFreeDir();

		cout("> %s\n", dir);

		addCwd(dir);
		coExecute_x(xcout("%s x%s \"%s\"", Get7zExeFile(), GetSwitches(), file7z));
		unaddCwd();
		Post7z(dir);
		dir = Unlittering(dir, file7z);
		execute_x(xcout("START %s", dir));
	}
	if (OutputAndDelete)
	{
		LOGPOS();
		removeFile(file7z);
	}
	memFree(dir);
	memFree(file7z);
}
int main(int argc, char **argv)
{
	cout("=======\n");
	cout("7z �W�J\n");
	cout("=======\n");

readArgs:
	if (argIs("/ANO"))
	{
		cout("ALLOW-NO-OUTPUT\n");

		AllowNoOutput = 1;
		goto readArgs;
	}
	if (argIs("/C"))
	{
		cout("+----------------+\n");
		cout("| �����ꏊ�ɓW�J |\n");
		cout("+----------------+\n");

		ExtractSameDir = 1;
		goto readArgs;
	}
	if (argIs("/T"))
	{
		cout("TRIM-ONE-DIR\n");

		TrimOneDir = 1;
		goto readArgs;
	}
	if (argIs("/OAD"))
	{
		cout("+-------------------+\n");
		cout("| OUTPUT AND DELETE |\n");
		cout("+-------------------+\n");

		OutputAndDelete = 1;
		goto readArgs;
	}
	if (argIs("/P"))
	{
		Passphrase = nextArg();
		goto readArgs;
	}

	if (hasArgs(1))
	{
		Extract7z(nextArg());
		return;
	}

	for (; ; )
	{
		Extract7z(c_dropFile());

		cout("\n");
		cout("-------\n");
		cout("7z �W�J\n");
		cout("-------\n");
	}
}
