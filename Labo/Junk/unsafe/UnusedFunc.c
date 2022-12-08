/*
	.sln がある場所から実行してね。

	.cpp .h ファイルを書き換える。
*/

#include "C:\Factory\Common\all.h"

static autoList_t *UnusedFuncFirstLines;

static char *Solution;
static char *ReleaseExeFile;

static autoList_t *Lines;
static uint FuncRangeBgn;
static uint FuncRangeEnd;

static int IsSuccessful(void)
{
	return existFile(ReleaseExeFile);
}
static int GetFuncRange(uint index, int headerFlag)
{
	char *line = getLine(Lines, index);

	if (headerFlag)
	{
		if (line[0] != '\t' || !m_iscsymf(line[1])) // ? C言語の識別子で始まっていない。
			return 0;
	}
	else
	{
		if (!m_iscsymf(line[0])) // ? C言語の識別子で始まっていない。
			return 0;
	}
	FuncRangeBgn = index;

	for (; ; )
	{
		index++;

		if (getCount(Lines) <= index) // ? ファイルの終端に到達
			return 0;

		line = getLine(Lines, index);

		if (headerFlag)
		{
			if (!strcmp(line, "\t{")) // ? 関数ブロック開始
				break;
		}
		else
		{
			if (!strcmp(line, "{")) // ? 関数ブロック開始
				break;
		}
		if (!line[0]) // ? 空行
			return 0;
	}
	for (; ; )
	{
		index++;

		if (getCount(Lines) <= index) // ? ファイルの終端に到達
			return 0;

		line = getLine(Lines, index);

		if (headerFlag)
		{
			if (!strcmp(line, "\t}")) // ? 関数ブロック終了
				break;
		}
		else
		{
			if (!strcmp(line, "}")) // ? 関数ブロック終了
				break;
		}
	}
	FuncRangeEnd = index;
	return 1;
}
static void CommentOutFunc_WriteFile(char *file)
{
	FILE *fp = fileOpen(file, "wt");
	char *line;
	uint index;

	foreach (Lines, line, index)
	{
		if (m_isRange(index, FuncRangeBgn, FuncRangeEnd))
		{
			line = "// DELETE";
		}
		writeLine(fp, line);
	}
	fileClose(fp);
}

static uint CommentedOutCount;

static void CommentOutFunc_Lines(void)
{
	char *line;
	uint index;

	foreach (Lines, line, index)
	{
		if (m_isRange(index, FuncRangeBgn, FuncRangeEnd))
		{
			line = replaceLine(line, "*/", "＊/", 0);

			setElement(Lines, index, (uint)xcout("// #### DELETED ==== %07u $$$$ //\t%s", CommentedOutCount, line));
			memFree(line);
		}
	}
	CommentedOutCount++;
}
static int CheckBuild(void)
{
	LOGPOS();
	coExecute("> C:\\temp\\1.tmp 2> C:\\temp\\2.tmp qq");
	coExecute("> C:\\temp\\1.tmp 2> C:\\temp\\2.tmp cx *");

	if (IsSuccessful())
	{
		LOGPOS();
		addElement(UnusedFuncFirstLines, (uint)strx(getLine(Lines, FuncRangeBgn)));
		return 1;
	}
	LOGPOS();
	return 0;
}
static void ProcCppFile(char *file, int headerFlag)
{
	char *escFile = addExt(strx(file), "{e0f86009-6dbe-49af-a1bd-ef8f20416f20}_UnusedFunc_Escape.txt");
	uint index;
	int commentedOut = 0;

	cout("ProcCppFile: %s %d\n", file, headerFlag);

	errorCase(existFile(escFile));
	moveFile(file, escFile);

	Lines = readLines(escFile);

	for (index = 0; index < getCount(Lines); index++)
	{
		if (GetFuncRange(index, headerFlag))
		{
			cout("%s\n", getLine(Lines, FuncRangeBgn));

			CommentOutFunc_WriteFile(file);

			if (CheckBuild())
			{
				CommentOutFunc_Lines();
				commentedOut = 1;
			}
			removeFile(file);
		}
	}
	if (commentedOut)
	{
		writeLines(file, Lines);
		removeFile(escFile);
	}
	else
	{
		moveFile(escFile, file);
	}
	releaseDim(Lines, 1);
	Lines = NULL;

	memFree(escFile);
	LOGPOS();
}
static void PrintResult(void)
{
	char *line;
	uint index;

	cout("RESULT >>>\n");

	foreach (UnusedFuncFirstLines, line, index)
		cout("%s\n", line);

	cout("<<< RESULT\n");
}
static void Main3(char *slnFile)
{
	autoList_t *files = lssFiles(".");
	char *file;
	uint index;

	// check
	{
		coExecute("qq");
		errorCase(IsSuccessful());
		coExecute("cx *");
		errorCase(!IsSuccessful());
		coExecute("qq");
		errorCase(IsSuccessful());
	}

	foreach (files, file, index)
	{
		if (!_stricmp("cpp", getExt(file)))
		{
			ProcCppFile(file, 0);
		}
		else if (!_stricmp("h", getExt(file)))
		{
			ProcCppFile(file, 0);
			ProcCppFile(file, 1);
		}
	}
	releaseDim(files, 1);
	coExecute("qq");

	PrintResult();
}
static void Main2(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;
	char *slnFile = NULL;

	foreach (files, file, index)
	{
		if (!_stricmp("sln", getExt(file)))
		{
			errorCase(slnFile); // ? 2つ目
			slnFile = file;
		}
	}
	errorCase(!slnFile);
	Solution = changeExt(getLocal(slnFile), "");
	ReleaseExeFile = xcout("Release\\%s.exe", Solution);
	Main3(slnFile);
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	UnusedFuncFirstLines = newList();

	Main2();
}
