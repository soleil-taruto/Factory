/*
	FOUNDLISTFILE ���̓J�����g�f�B���N�g���̔z���ɂ���u�������e�v�̃t�@�C����ʂ́u�������e�v�ɕҏW����B

	--
	�R�}���h

	ees.exe /LSS

		FOUNDLISTFILE �̃t�@�C����ҏW����B

	ees.exe BASE-FILE

		�J�����g�f�B���N�g���̔z���ɂ��� BASE-FILE �Ɠ������e�̃t�@�C����ҏW����B
*/

#include "C:\Factory\Common\all.h"


static void EditSame(char *baseFile, autoList_t *files)
{
	char *escBaseFile = makeTempPath(NULL);
	char *file;
	uint index;

	cout("< %s\n", baseFile);

	foreach(files, file, index)
		cout("> %s\n", file);

	copyFile(baseFile, escBaseFile);

	editTextFile(baseFile);

	if(!existFile(baseFile)) // ? �G�ۂɂ���č폜���ꂽ�B-> ����
		copyFile(escBaseFile, baseFile);

	if(!isSameFile(baseFile, escBaseFile))
	{
		cout("�ҏW��K�p���܂��B\n");
		cout("���s�H\n");

		if(clearGetKey() == 0x1b)
			termination(0);

		cout("���s���܂��B\n");

		foreach(files, file, index)
		{
			cout("< %s\n", baseFile);
			cout("> %s\n", file);

			semiRemovePath(file);

			copyFile(baseFile, file);
		}
		cout("apply-done\n");
	}
	removeFile(escBaseFile);
	memFree(escBaseFile);
}
static void EditSame_LSS(void)
{
	autoList_t *files = readLines(FOUNDLISTFILE);
	char *file;
	uint index;
	char *baseFile;
	int hasDifferentFile = 0;

	errorCase(getCount(files) < 1);

	sortJLinesICase(files);
	baseFile = (char *)desertElement(files, 0);

	cout("baseFile: %s\n", baseFile);

	errorCase(!existFile(baseFile));

	foreach(files, file, index)
	{
		errorCase(!mbs_stricmp(file, baseFile));
		errorCase(!existFile(file));

		if(!isSameFile(file, baseFile))
		{
			cout("NOT_SAME: %s\n", file);
			hasDifferentFile = 1;
		}
	}

	if(hasDifferentFile)
	{
		cout("���e�̈قȂ�t�@�C�����܂܂�Ă��܂��B\n");
		cout("���s�H\n");

		if(clearGetKey() == 0x1b)
			termination(0);

		cout("���s���܂��B\n");
	}

	EditSame(baseFile, files);

	releaseDim(files, 1);
	memFree(baseFile);
}
static void EditSame_File(char *baseFile)
{
	autoList_t *files = lssFiles(".");
	char *file;
	uint index;

	baseFile = makeFullPath(baseFile);

	errorCase_m(findJLineICase(files, baseFile) == getCount(files), "�w�肳�ꂽ�t�@�C���̓J�����g�f�B���N�g���̔z���ɍ݂�܂���B");

	foreach(files, file, index)
		if(!mbs_stricmp(file, baseFile) || !isSameFile(file, baseFile)) // ? baseFile �ł��� || baseFile �Ɠ��e���قȂ�B-> ���O
			*file = '\0';

	trimLines(files);
	sortJLinesICase(files);

	EditSame(baseFile, files);

	releaseDim(files, 1);
	memFree(baseFile);
}
int main(int argc, char **argv)
{
	antiSubversion = 1;

	if(argIs("/LSS"))
	{
		EditSame_LSS();
	}
	else
	{
		EditSame_File(nextArg());
	}
}
