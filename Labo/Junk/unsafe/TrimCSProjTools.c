/*
	Tools\ 直下の使用してない .cs を削除する。

	----

	.sln がある場所から実行してね。(C#)

	プロジェクト直下の Tools\ 直下の .cs を書き換えます。

	.cs を削除するだけです。
	プロジェクトからの除去は手動で行って下さい。

	.exe のプロジェクトじゃなければならない。<---- Tools があるなら多分 .exe でしょう。
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
				moveFile(escapedFile, file); // 復元
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
			errorCase(slnFile); // ? 2つ目
			slnFile = strx(file);
		}
	}
	errorCase(!slnFile); // ? 1つも無い。

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
