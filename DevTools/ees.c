/*
	FOUNDLISTFILE ���̓J�����g�f�B���N�g���̔z���ɂ���u�������e�v�̃t�@�C����ʂ́u�������e�v�ɕҏW����B

	--
	�R�}���h

	ees.exe /LSS [/E]

		FOUNDLISTFILE �̃t�@�C����ҏW����B

		/E ... �ǂ̃t�@�C����ҏW�ΏۂƂ��邩���[�U�[�I���Ƃ���B

	ees.exe BASE-FILE

		�J�����g�f�B���N�g���̔z���ɂ��� BASE-FILE �Ɠ������e�̃t�@�C����ҏW����B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

static int S_HasDifferentFile = 0;

static void EditSame(char *baseFile, autoList_t *files)
{
	char *escBaseFile = makeTempPath(NULL);
	char *file;
	uint index;

	cout("< %s\n", baseFile);

	foreach (files, file, index)
		cout("> %s\n", file);

	copyFile(baseFile, escBaseFile);

	editTextFile(baseFile);

	if (!existFile(baseFile)) // ? �G�ۂɂ���č폜���ꂽ�B-> ����
		copyFile(escBaseFile, baseFile);

	if (!isSameFile(baseFile, escBaseFile) || S_HasDifferentFile)
	{
		cout("�ҏW��K�p���܂��B%d\n", S_HasDifferentFile);
		cout("���s�H\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("���s���܂��B\n");

		foreach (files, file, index)
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
static void EditSame_LSS(int userSelectMode)
{
	autoList_t *files = readLines(FOUNDLISTFILE);
	char *file;
	uint index;
	char *file_2;
	uint index_2;
	uint baseFileIndex;
	char *baseFile;
	int hasDifferentFile = 0;

	// ---- Check ----

	errorCase(getCount(files) < 1);

	foreach (files, file, index)
		errorCase(!existFile(file));

	foreach (files, file, index)
	foreach (files, file_2, index_2)
		errorCase(index < index_2 && !mbs_stricmp(file, file_2));

	// ----

	sortJLinesICase(files);

	if (userSelectMode)
	{
		baseFile = selectLine(files);

		if (!baseFile)
			termination(0);

		baseFileIndex = findLine(files, baseFile);
		errorCase(baseFileIndex == getCount(files));
		memFree(baseFile);
		baseFile = NULL;
	}
	// �x�[�X�t�@�C���I�сF
	// 1�t�@�C���݈̂���Ă���(�ҏW�ς�)�ŁA����ȊO����(���ҏW)�ł���ꍇ��z�肵�āA
	// �ҏW�ς݂̃t�@�C�����̗p����B
	//
	else if (3 <= getCount(files))
	{
		for (index = 1; index < getCount(files); index++)
			if (!isSameFile(getLine(files, 0), getLine(files, index)))
				break;

		if (index == 1)
			baseFileIndex = isSameFile(getLine(files, 0), getLine(files, 2)) ? 1 : 0;
		else if (index == getCount(files))
			baseFileIndex = 0;
		else
			baseFileIndex = index;
	}
	else
	{
		baseFileIndex = 0;
	}
	baseFile = (char *)desertElement(files, baseFileIndex);
	cout("%s %s <�ҏW�Ώ�>\n", c_md5_makeHexHashFile(baseFile), baseFile);

	foreach (files, file, index)
	{
		cout("%s %s\n", c_md5_makeHexHashFile(file), file);

		if (!isSameFile(file, baseFile))
			hasDifferentFile = 1;
	}
	if (hasDifferentFile)
	{
		cout("���e�̈قȂ�t�@�C�����܂܂�Ă��܂��B\n");
		cout("���s�H\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("���s���܂��B\n");
	}
	S_HasDifferentFile = hasDifferentFile;

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

	foreach (files, file, index)
		if (!mbs_stricmp(file, baseFile) || !isSameFile(file, baseFile)) // ? baseFile �ł��� || baseFile �Ɠ��e���قȂ�B-> ���O
			*file = '\0';

	trimLines(files);
	sortJLinesICase(files);

	// write to FOUNDLISTFILE
	{
		FILE *fp = fileOpen(FOUNDLISTFILE, "wt");

		writeLine(fp, baseFile);
		writeLines2Stream(fp, files);

		fileClose(fp);
	}

	EditSame(baseFile, files);

	releaseDim(files, 1);
	memFree(baseFile);
}
int main(int argc, char **argv)
{
	antiSubversion = 1;

	if (argIs("/LSS"))
	{
		EditSame_LSS(argIs("/E"));
	}
	else
	{
		EditSame_File(nextArg());
	}
}
