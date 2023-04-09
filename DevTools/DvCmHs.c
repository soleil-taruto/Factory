/*
	display Dev *Commons dir Hash

	����𔭌������ꍇ�A�ŏ��ɔ�����������̂���t�@�C���Z�b�g�� FOUNDLISTFILE �֏����o���B
	�A���A���Ⴊ�t�@�C���̗L���ł������ꍇ FOUNDLISTFILE �͍폜�����B
*/

#include "C:\Factory\Common\all.h"

static autoList_t *CommonsDirNames;
static autoList_t *CommonsDirsList;
static int ExistDiffOverall;

static int IsCommonsDir(char *dir)
{
	return
		endsWithICase(dir, "Commons") ||
		!_stricmp(getLocal(dir), "Drawings") ||
		!_stricmp(getLocal(dir), "WebServices");
}
static void AddCommonsDirsIfExist(char *projDir)
{
	autoList_t *dirs = slsDirs(projDir);
	char *dir;
	uint index;

	foreach (dirs, dir, index)
	{
		if (IsCommonsDir(dir))
		{
			char *name = getLocal(dir);
			uint namePos;

			cout("< %s\n", dir);
			cout("> %s\n", name);

			namePos = findLineCase(CommonsDirNames, name, 1);

			if (namePos == getCount(CommonsDirNames))
			{
				addElement(CommonsDirNames, (uint)strx(name));
				addElement(CommonsDirsList, (uint)newList());
			}
			addElement(getList(CommonsDirsList, namePos), (uint)strx(dir));
		}
	}
	releaseDim(dirs, 1);
}
static char *GetDirMD5(char *dir)
{
	char *rdrFile = makeTempPath("tmp");
	char *hash;

	execute_x(xcout("C:\\Factory\\Tools\\dmd5.exe /S \"%s\" > \"%s\"", dir, rdrFile));

	hash = readFirstLine(rdrFile);

	errorCase(!lineExp("<32,09af>", hash));

	removeFile(rdrFile);
	memFree(rdrFile);

	return hash;
}
static void OneDiffFileSetToFoundFileList(autoList_t *commonsDirs)
{
	char *firstCommonsDir;
	autoList_t *files;
	char *file;
	uint index;

	LOGPOS();

	// ���炩���ߍ폜���Ă����B
	// �����̂���t�@�C����������Ȃ��ꍇ�͍폜���ꂽ�܂܂ɂȂ�B
	// -- �f�B���N�g���̍������t�@�C���̗L���ł���ꍇ
	removeFileIfExist(FOUNDLISTFILE);

	firstCommonsDir = getLine(commonsDirs, 0);
	files = slssFiles(firstCommonsDir);
	changeRoots(files, firstCommonsDir, NULL);

	foreach (files, file, index)
	{
		char *commonsDir;
		uint commonsDirIndex;

		foreach (commonsDirs, commonsDir, commonsDirIndex)
		if (commonsDirIndex)
		{
			char *file1 = combine(firstCommonsDir, file);
			char *file2 = combine(commonsDir, file);
			int existAndDiff;

			existAndDiff = existFile(file2) && !isSameFile(file1, file2);

			memFree(file1);
			memFree(file2);

			if(existAndDiff) // ? ���Ⴀ��
				break;
		}

		if (commonsDirIndex < getCount(commonsDirs)) // ? ����𔭌������B-> ���Ⴀ��t�@�C���Z�b�g�� FOUNDLISTFILE �֏����o���B
		{
			FILE *fp = fileOpen(FOUNDLISTFILE, "wt");

			foreach (commonsDirs, commonsDir, commonsDirIndex)
				writeLine_x(fp, combine(commonsDir, file));

			fileClose(fp);
			LOGPOS();
			break;
		}
	}
	releaseDim(files, 1);

	LOGPOS();
}
static void ShowCommonsDirs(char *name, autoList_t *commonsDirs)
{
	char *dir;
	uint index;
	char *firstHash = NULL;
	int existDiff = 0;

	cout("\n");
	cout("----\n");
	cout("%s\n", name);

	foreach (commonsDirs, dir, index)
	{
		char *hash = GetDirMD5(dir);

		cout("%s %s\n", hash, dir);

		if (firstHash)
			existDiff |= _stricmp(firstHash, hash);
		else
			firstHash = strx(hash);

		memFree(hash);
	}
	memFree(firstHash);

	if (existDiff)
	{
		cout("+----------+\n");
		cout("| ���Ⴀ�� |\n");
		cout("+----------+\n");

		if (!ExistDiffOverall)
		{
			OneDiffFileSetToFoundFileList(commonsDirs);
			ExistDiffOverall = 1;
		}
	}
}
static void ShowAllCommonsDirs(void)
{
	char *name;
	uint index;

	foreach (CommonsDirNames, name, index)
	{
		ShowCommonsDirs(name, getList(CommonsDirsList, index));
	}

	if (ExistDiffOverall)
	{
		cout("\n");
		cout("#####################\n");
		cout("## 1���ȏ㑊�Ⴀ�� ##\n");
		cout("#####################\n");
	}
}
int main(int argc, char **argv)
{
	autoList_t *dirs = slsDirs("C:\\Dev");
	char *dir;
	uint index;

	CommonsDirNames = newList();
	CommonsDirsList = newList();

	foreach (dirs, dir, index)
	{
		autoList_t *files = lsFiles(dir);
		char *file;
		uint index;

		foreach (files, file, index)
			if (!_stricmp(getExt(file), "csproj"))
				break;

		if (index < getCount(files)) // ? �v���W�F�N�gDIR
		{
			AddCommonsDirsIfExist(dir);
		}
		else // ? not �v���W�F�N�gDIR
		{
			addElements_x(dirs, slsDirs(dir));
		}
		releaseDim(files, 1);
	}
	releaseDim(dirs, 1);

	ShowAllCommonsDirs();
}
