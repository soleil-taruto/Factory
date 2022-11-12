/*
	UpdatingCopy.exe ���͑��f�B���N�g�� �o�͑��f�B���N�g��

		�o�͑��f�B���N�g��

			"*(P)" �����[�g�f�B���N�g�����猩�đ�Q�K�w�̃f�B���N�g���̃��[�J�����ɒu��������B��F"C:\\aaaa\\bbbb\\cccc" -> bbbb
			"*(C)" ���J�����g�f�B���N�g���̃��[�J�����ɒu��������B
*/

#include "C:\Factory\Common\all.h"

static char *x_DirFilter(char *dir)
{
	autoList_t *pTkns = tokenizeYen_x(getCwd());

	errorCase(getCount(pTkns) < 4); // "C:\\Dev\\PPPP\\x99999999_CCCC" �Ɠ������A��������[���Ȃ���΂Ȃ�Ȃ��B

	dir = strx(dir);

	dir = replaceLine(dir, "*(P)", getLine(pTkns, 2), 0); // ��2�K�w�̃��[�J����
	dir = replaceLine(dir, "*(C)", getLine(pTkns, getCount(pTkns) - 1), 0); // �J�����gDIR�̃��[�J����

	releaseDim(pTkns, 1);

	return dir;
}
static void CheckWDir(char *wDir)
{
	if (!existDir(wDir))
	{
		cout("�o�͑��f�B���N�g�����쐬���܂��B\n");
		cout("wDir: %s\n", wDir);

		createDir(wDir);
	}
}
static void RemoveOldZip(char *wDir)
{
	autoList_t *files = lsFiles(wDir);
	char *file;
	uint index;

	foreach (files, file, index)
		if (!_stricmp(getExt(file), "zip"))
			removeFile(file);

	releaseDim(files, 1);
}
static char *GetFirstZipFile(char *rDir)
{
	autoList_t *files = lsFiles(rDir);
	char *file;
	uint index;

	foreach (files, file, index)
		if (!_stricmp(getExt(file), "zip"))
			break;

	errorCase_m(!file, ".zip �t�@�C����������܂���B");

	file = strx(file);
	releaseDim(files, 1);
	return file;
}
static void CopyZip(char *rDir, char *wDir)
{
	char *rFile = GetFirstZipFile(rDir);
	char *wFile;

	wFile = combine(wDir, getLocal(rFile));

	copyFile(rFile, wFile);

	memFree(rFile);
	memFree(wFile);
}
int main(int argc, char **argv)
{
	char *rDir;
	char *wDir;
	char *oldPrefix;

	rDir = nextArg();
	wDir = nextArg();

	wDir = x_DirFilter(wDir);

	cout("< %s\n", rDir);
	cout("> %s\n", wDir);

	CheckWDir(wDir);

	errorCase(!existDir(rDir));
	errorCase(!existDir(wDir)); // 2bs

	LOGPOS();
	RemoveOldZip(wDir);
	LOGPOS();
	CopyZip(rDir, wDir);
	LOGPOS();

	memFree(wDir);
}
