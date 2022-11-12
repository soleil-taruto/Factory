/*
	Tools\ �����̎g�p���ĂȂ� .cs ���폜����B

	----

	.sln ������ꏊ������s���ĂˁB(C#)

	�v���W�F�N�g������ Tools\ ������ .cs �����������܂��B

	.cs ���폜���邾���ł��B
	�v���W�F�N�g����̏����͎蓮�ōs���ĉ������B

	.exe �̃v���W�F�N�g����Ȃ���΂Ȃ�Ȃ��B<---- Tools ������Ȃ瑽�� .exe �ł��傤�B
*/

#include "C:\Factory\Common\all.h"

#define DIR_TOOLS "Tools"

static char *ProjName;
static char *BuiltExeFile;

static int TryBuild_NoCD(void)
{
	coExecute("qq");

	errorCase(existFile(BuiltExeFile));

	coExecute("cx **");

	return existFile(BuiltExeFile);
}
static int TryBuild(void)
{
	int ret;

	addCwd("..\\..");
	{
		ret = TryBuild_NoCD();
	}
	unaddCwd();

	return ret;
}
static void Main2_Tools(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;
	autoList_t *deletableCsFiles = newList();
	char *escapedFile = makeTempPath(NULL);
	int deletedFlag;

LOGPOS();
restart:
LOGPOS();
	errorCase(!TryBuild());

	deletedFlag = 0;

LOGPOS();
	foreach (files, file, index)
	{
		if (!_stricmp("cs", getExt(file)))
		{
			moveFile(file, escapedFile);

			writeOneLine(file, "// try delete");

			if (TryBuild())
			{
				addElement(deletableCsFiles, (uint)strx(file));
				*file = '\0';
				deletedFlag = 1;
				removeFile(escapedFile);
			}
			else
			{
				removeFile(file);
				moveFile(escapedFile, file); // ����
			}
		}
	}
LOGPOS();
	trimLines(files);
LOGPOS();

	if (deletedFlag)
		goto restart;

LOGPOS();
	foreach (deletableCsFiles, file, index)
	{
		removeFile(file);
	}
LOGPOS();
	releaseDim(files, 1);
	releaseDim(deletableCsFiles, 1);
	memFree(escapedFile);
LOGPOS();
}
static void Main2_Project(void)
{
	char *csProjFile = xcout("%s.csproj", ProjName);

	errorCase(!existFile(csProjFile));
	errorCase(!existDir(DIR_TOOLS));

	addCwd(DIR_TOOLS);
	{
		Main2_Tools();
	}
	unaddCwd();
}
static void Main2_Solution(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;
	char *slnFile = NULL;
	char *projName;

	foreach (files, file, index)
	{
		if (!_stricmp("sln", getExt(file)))
		{
			errorCase(slnFile); // ? 2��
			slnFile = strx(file);
		}
	}
	errorCase(!slnFile); // ? 1�������B

	ProjName = changeExt(getLocal(slnFile), "");
	BuiltExeFile = combine_xx(getCwd(), xcout("%s\\bin\\Release\\%s.exe", ProjName, ProjName));

	errorCase(!existDir(ProjName));

	addCwd(ProjName);
	{
		Main2_Project();
	}
	unaddCwd();

	releaseDim(files, 1);
	memFree(slnFile);
}
int main(int argc, char **argv)
{
LOGPOS();
	Main2_Solution();
LOGPOS();
}
