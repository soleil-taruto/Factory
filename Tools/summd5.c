/*
	summd5.exe [/-E] [/M] ...

		/-E ... �`�F�b�N�̂Ƃ��s��v�ł��G���[��~���Ȃ��B
		/M  ... �f�B���N�g������������Ƃ��A�n�b�V���̖����t�@�C���������i�n�b�V���쐬�j����B

	summd5.exe ... FILE

		( { FILE } .md5 ) �����݂��� -> �`�F�b�N ( FILE <- { FILE } .md5 )
		( { FILE } .md5 ) �����݂��Ȃ� -> ���� ( FILE -> { FILE } .md5 )

	summd5.exe ... DIR

		�z���̑S�Ă� FILE, ( { FILE } .md5 ) �̑g�ݍ��킹�ɂ��āA�`�F�b�N����B(�f�t�H���g�Ő����͂��Ȃ�)

	summd5.exe ...

		drag and drop, FILE or DIR
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

#define SUM_EXT "md5"

static int ResumeInvalidSum;
static int AutoMakeMode;

static void ProcFileSum(char *trgFile, char *sumFile)
{
	cout("< %s\n", trgFile);
	cout("> %s\n", sumFile);

	errorCase(!existFile(trgFile));

	if (existFile(sumFile))
	{
		char *trgHash;
		char *sumHash;

		cout("CHECK\n");

		trgHash = md5_makeHexHashFile(trgFile);
		sumHash = readFirstLine(sumFile);

		cout("<* %s\n", trgHash);
		cout(">* %s\n", sumHash);

		if (!_stricmp(trgHash, sumHash))
		{
			cout("CHECK OK\n");
		}
		else
		{
			errorCase(!ResumeInvalidSum);

			cout("******************\n");
			cout("* FILE IS BROKEN *\n");
			cout("******************\n");
		}
	}
	else
	{
		cout("MAKE SUM\n");
		writeOneLine(sumFile, c_md5_makeHexHashFile(trgFile));
		cout("MAKE OK\n");
	}
}
static void ProcFile(char *file)
{
	char *trgFile;
	char *sumFile;

	if (!_stricmp(SUM_EXT, getExt(file))) // == SUM_EXT
	{
		trgFile = changeExt(file, "");
		sumFile = strx(file);
	}
	else
	{
		trgFile = strx(file);
		sumFile = addExt(strx(file), SUM_EXT);
	}

	ProcFileSum(trgFile, sumFile);

	memFree(trgFile);
	memFree(sumFile);
}
static void ProcDir(char *dir)
{
	autoList_t *files = lssFiles(dir);
	char *trgFile;
	uint index;

	foreach (files, trgFile, index)
	{
		if (_stricmp(SUM_EXT, getExt(trgFile))) // != SUM_EXT
		{
			char *sumFile = addExt(strx(trgFile), SUM_EXT);

			if (existFile(sumFile) || AutoMakeMode)
			{
				ProcFileSum(trgFile, sumFile);
			}
			memFree(sumFile);
		}
	}
	releaseDim(files, 1);

	cout("CHECK DIR OK\n");
}
static void ProcMain(char *path)
{
	if (existFile(path))
	{
		ProcFile(path);
	}
	else if (existDir(path))
	{
		ProcDir(path);
	}
	else
	{
		error();
	}
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/-E"))
	{
		ResumeInvalidSum = 1;
		goto readArgs;
	}
	if (argIs("/M"))
	{
		AutoMakeMode = 1;
		goto readArgs;
	}

	if (hasArgs(1))
	{
		ProcMain(nextArg());
	}
	else
	{
		for (; ; )
		{
			ProcMain(c_dropDirFile());
			cout("\n");
		}
	}
}
