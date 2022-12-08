/*
	Tools\ 直下の .cs 内の使用してないメンバ (メソッド・フィールド・プロパティ) を削除する。

	----

	.sln がある場所から実行してね。(C#)

	プロジェクト直下の Tools\ 直下の .cs を書き換えます。

	.cs 内のメンバ (メソッド・フィールド・プロパティ) を削除します。
	.cs は削除しません。

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

static uint IM_StartLineIndex;
static uint IM_EndLineIndex;

/*
	クラスメンバの宣言部であるかチェックする。

	クラスメンバの宣言部であった場合、以下を設定する。
		IM_StartLineIndex  <--  当該メンバの開始行インデックス
		IM_EndLineIndex    <--  当該メンバの最終行インデックス + 1

	lines: ソース
	targLineIndex: チェックする行インデックス
	ret: ? クラスメンバの宣言部である。
*/
static int IsMember(autoList_t *lines, uint targLineIndex)
{
	char *line = getLine(lines, targLineIndex);
	uint index;
	int status;

	status =
		startsWith(line, "\t\tpublic ") ||
		startsWith(line, "\t\tprivate ") ||
		startsWith(line, "\t\tprotected ");

	if (!status)
		return 0;

	// 上端を検索 ...

	for (index = targLineIndex; index; index--)
	{
		line = getLine(lines, index - 1);
		status =
			startsWith(line, "\t\t[") ||
			startsWith(line, "\t\t/// ");

		if (!status)
			break;
	}
	IM_StartLineIndex = index;

	// 下端を検索 ...

	index = targLineIndex;
	index++;
	line = refLine(lines, index);

	if (startsWith(line, "\t\t\t: "))
	{
		index++;
		line = refLine(lines, index);

		if (strcmp(line, "\t\t{ }"))
			return 0;

		IM_EndLineIndex = index + 1;
		return 1;
	}
	if (!strcmp(line, "\t\t{ }"))
	{
		IM_EndLineIndex = index + 1;
		return 1;
	}
	if (strcmp(line, "\t\t{"))
	{
		IM_EndLineIndex = index;
		return 1;
	}
	for (; ; )
	{
		index++;

		if (getCount(lines) <= index)
			return 0;

		line = getLine(lines, index);

		if (!strcmp(line, "\t\t}"))
			break;

		status =
			!*line ||
			startsWith(line, "#") ||
			startsWith(line, "\t\t\t");

		if (!status)
			return 0;
	}
	IM_EndLineIndex = index + 1;
	return 1;
}
static void EraseMember(autoList_t *lines, uint startLineIndex, uint endLineIndex)
{
	uint index;

	for (index = startLineIndex; index < endLineIndex; index++)
		*getLine(lines, index) = '\0';
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
			autoList_t *lines = readLines(file); // ソースファイルから読み込み
			char *line;
			uint line_index;

			foreach (lines, line, line_index)
			{
				if (IsMember(lines, line_index))
				{
					autoList_t *emLines = copyLines(lines);
					uint startLineIndex = IM_StartLineIndex;
					uint endLineIndex   = IM_EndLineIndex;

					cout("TRY-ERASE-MEMBER.1\n");

					cout("TEM_file: %s\n", file);
					cout("TEM_stLn: %u\n", startLineIndex);
					cout("TEM_edLn: %u\n", endLineIndex);

					EraseMember(emLines, startLineIndex, endLineIndex);

					writeLines(file, emLines);
copyFile_cx(file, toCreatableTildaPath(combine("C:\\temp", getLocal(file)), IMAX)); // test test test test test

					if (TryBuild())
					{
						LOGPOS();
						releaseDim(lines, 1);
						lines = emLines;
						deletedFlag = 1;
					}
					else
					{
						releaseDim(emLines, 1);
					}

					cout("TRY-ERASE-MEMBER.2\n");
				}
			}
			writeLines(file, lines); // ソースファイルを復元する。

			releaseDim(lines, 1);
		}
	}
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
