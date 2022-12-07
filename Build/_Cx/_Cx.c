#include "C:\Factory\Common\all.h"
#include "libs\SolutionOrder.h"

#define CLSTDERR "_Cx_e.tmp"
#define CLSTDOUT "_Cx_o.tmp"

#define CLOPTIONS "/W2 /w24013 /WX /Oxt /J /GF"

#define MSBUILDOPTIONS "/property:configuration=Release"

#define BUILT_TIME_MARGIN 0

enum
{
	OPTLV_DEFAULT,
	OPTLV_LINK_TEST_MODULE,
	CLEANING_MODE,
};

static int OptimizeLevel = OPTLV_DEFAULT;
static int ForceBuildMode;

static uint BuiltObjCount;
static uint BuiltExeCount;
static uint BuiltSlnCount;
static uint SkippedObjCount;
static uint SkippedExeCount;
static autoList_t *BuiltLines;

static char *NewestFile;
static time_t NewestFileTime;

static void ClearNewestFile(void)
{
	memFree(NewestFile);

	NewestFile = NULL;
	NewestFileTime = -1;
}
static void UpdateNewestFile(char *file)
{
	time_t fileTime = getFileModifyTime(file);

	if (NewestFileTime < fileTime)
	{
		memFree(NewestFile);

		NewestFile = strx(file);
		NewestFileTime = fileTime;
	}
}

// ---- error message ----

static autoList_t *ExitMessages;

static void ShowExitMessages(void)
{
	char *message;
	uint index;

	foreach (ExitMessages, message, index)
	{
		cout("%s\n", message);
	}
}
static void AddExitMessage_x(char *message)
{
	if (!ExitMessages)
	{
		ExitMessages = newList();
		addFinalizer(ShowExitMessages);
	}
	addElement(ExitMessages, (uint)message);
}

// ----

static autoList_t *GetResponse(char *source, int buildFlag)
{
	autoList_t *remFiles = newList();
	autoList_t *includes = newList();
	autoList_t *response = newList();
	char *file;
	uint index;

	source = makeFullPath(source);
	addElement(remFiles, (uint)source);

	ClearNewestFile();
	UpdateNewestFile(source);

	foreach (remFiles, file, index)
	{
		autoList_t *lines = readLines(file);
		char *line;
		uint line_index;
		char *dir = getParent(file);

		addCwd(dir);

		foreach (lines, line, line_index)
		{
			if (lineExp("<\t\t  >#<\t\t  >include<\t\t  >\"<>\"<>", line))
			{
				char *p = strchr(line, '\"');
				char *q;

				errorCase(!p);
				p++;
				q = strchr(p, '\"');
				errorCase(!q);
				*q = '\0';

				if (!existFile(p))
				{
					cout("存在しないインクルード先: %s -> %s\n", file, p);
					continue; // #if 0 の中の可能性があるので、エラーにせず続行する。
				}
				p = makeFullPath(p);

				if (findLineCase(includes, p, 1) == getCount(includes))
				{
					addElement(remFiles, (uint)p);
					addElement(includes, (uint)strx(p));
				}
				else
					memFree(p);
			}
		}
		unaddCwd();
		memFree(dir);
		releaseDim(lines, 1);
	}
	releaseDim(remFiles, 1);
	sortJLinesICase(includes);

	if (buildFlag)
	{
		foreach (includes, file, index)
		{
			char *objfile = changeExt(file, "obj");

			if (existFile(objfile))
			{
				if (OptimizeLevel == OPTLV_LINK_TEST_MODULE)
				{
					char *testobjfile = xcout("%sTest.obj", c_changeExt(file, ""));

					if (existFile(testobjfile))
					{
						cout("< %s\n", objfile);
						cout("> %s\n", testobjfile);

						memFree(objfile);
						objfile = testobjfile;
					}
					else
						memFree(testobjfile);
				}
				addElement(response, (uint)objfile);
			}
			else
				memFree(objfile);
		}
	}
	foreach (includes, file, index)
	{
		if (existFile(file))
		{
			UpdateNewestFile(file);
		}
	}
	foreach (response, file, index)
	{
		UpdateNewestFile(file);
	}
	releaseDim(includes, 1);
	return response;
}
static void MakeResponse(char *source, char *response, int buildFlag)
{
	autoList_t *objfiles = GetResponse(source, buildFlag);

	if (buildFlag)
		writeLines(response, objfiles);

	releaseDim(objfiles, 1);
}
static uint GetVSEditionYear(char *slnFile)
{
	FILE *fp = fileOpen(slnFile, "rt");
	char *line;
	uint ret;

	memFree(neReadLine(fp));
	memFree(neReadLine(fp));
	line = neReadLine(fp);

	if (lineExp("# Visual <1,,\x21\x7e> Express 2008", line))
	{
		ret = 2008;
	}
	else if (lineExp("# Visual <1,,\x21\x7e> Express 2010", line))
	{
		ret = 2010;
	}
	else if (!strcmp(line, "# Visual Studio Version 16"))
	{
		ret = 2019;
	}
	else if (!strcmp(line, "# Visual Studio Version 17"))
	{
		ret = 2022;
	}
	else
	{
		error_m("Unknown VS Edition");
	}

	memFree(line);
	fileClose(fp);
	return ret;
}
static int IsGitMaskedSource(char *source)
{
	FILE *fp = fileOpen(source, "rt");
	char *line;
	char *p;

	while (line = readLine(fp))
	{
		for (p = line; *p && *p <= ' '; ) // 行頭のインデント等の空白をスキップ
			p++;

		if (*p && (p[0] != '/' || p[1] != '/')) // ? ! (空行 || "//" で始まる行)
		{
			memFree(line);
			fileClose(fp);
			return 0;
		}
		memFree(line);
	}
	fileClose(fp);
	return 1;
}

static void Build(char *module, uint remCount) // remCount: 0 == 無効
{
	char *source = changeExt(module, "c");
	char *header = changeExt(module, "h");
	char *objfile = changeExt(module, "obj");
	char *exefile = changeExt(module, "exe");
	char *report = changeExt(module, "report");
	char *response = changeExt(module, "response");
	char *solution = changeExt(module, "sln");
	char *exemanifest;
	int objmode;

	exemanifest = addExt(strx(exefile), "manifest");

//	if (!existFile(source))
//		source = addLine(source, "pp"); // .c -> .cpp

	errorCase(mbs_strchr(module, '\\')); // ローカル名であること。
	errorCase(strchr(module, ' ')); // コマンドラインに渡すため、空白を含まないこと。

	if (existFile(solution))
	{
		if (OptimizeLevel != CLEANING_MODE)
		{
			uint vsEditionYear = GetVSEditionYear(solution);

			cout("solution file's Visual Studio Editon (Year) == %u\n", vsEditionYear);

			if (vsEditionYear == 2010 || vsEditionYear == 2019)
			{
				char *slncacheFile = addExt(strx(solution), "cache");
				int successful;

				execute_x(xcout("MSBUILD " MSBUILDOPTIONS " %s", solution));

				successful = lastSystemRet == 0;

				if (existFile(slncacheFile))
				{
					cout("%s\n", slncacheFile);
					cout("sln.cacheファイルが存在します。ビルドは失敗しました。\n");
					successful = 0;
				}
				if (successful) // ? ビルド成功
				{
					// noop
				}
				else // ? ビルド失敗
				{
					cout("%s\\%s", c_getCwd(), solution);
					termination(1);
				}
				memFree(slncacheFile);

				BuiltSlnCount++;
				addElement(BuiltLines, (uint)xcout("BUILT_SLN %s", solution));
			}
		}
		goto endfunc;
	}
	errorCase(!existFile(source));

//	remove(objfile); // moved
//	remove(exefile); // moved
	remove(report);
	remove(response);
	remove(exemanifest);
	remove(CLSTDERR);
	remove(CLSTDOUT);

	if (OptimizeLevel == CLEANING_MODE)
	{
		remove(objfile);
		remove(exefile);

		goto endfunc;
	}
	objmode = existFile(header);

	if (objmode)
	{
		if (!ForceBuildMode && existFile(objfile))
		{
			time_t objfileTime = getFileModifyTime(objfile);

			MakeResponse(source, response, 0);

			cout("NewestFile=%s\n", NewestFile);
			cout("NewestFileTime: %I64d, objfileTime: %I64d\n", NewestFileTime, objfileTime);

			if (NewestFileTime + BUILT_TIME_MARGIN <= objfileTime) // ? 最後の修正・コンパイルの後でコンパイルされている。
			{
				cout("SKIP_BUILD_OBJ\n");
				SkippedObjCount++;
				addElement(BuiltLines, (uint)xcout("SKIPPED_OBJ %s", objfile));
				goto endfunc;
			}
		}
		remove(objfile);
		remove(exefile);

		if (IsGitMaskedSource(source))
		{
			cout("SKIP_MASKED_OBJ\n");
			SkippedObjCount++;
			addElement(BuiltLines, (uint)xcout("SKIPPED_OBJ %s (MASKED)", objfile));
			goto endfunc;
		}

		execute_x(xcout("2> " CLSTDERR " > " CLSTDOUT " CL " CLOPTIONS " /c %s", source));

		BuiltObjCount++;
		addElement(BuiltLines, (uint)xcout("BUILT_OBJ %s", objfile));
	}
	else
	{
		MakeResponse(source, response, 1);

		if (!ForceBuildMode && existFile(exefile))
		{
			time_t exefileTime = getFileModifyTime(exefile);

			cout("NewestFile=%s\n", NewestFile);
			cout("NewestFileTime: %I64d, exefileTime: %I64d\n", NewestFileTime, exefileTime);

			if (NewestFileTime + BUILT_TIME_MARGIN <= exefileTime) // ? 最後の修正・コンパイルの後でビルドされている。
			{
				removeFile(response);

				cout("SKIP_BUILD_EXE\n");
				SkippedExeCount++;
				addElement(BuiltLines, (uint)xcout("SKIPPED_EXE %s", exefile));
				goto endfunc;
			}
		}
		remove(objfile);
		remove(exefile);

		if (IsGitMaskedSource(source))
		{
			cout("SKIP_MASKED_EXE\n");
			SkippedExeCount++;
			addElement(BuiltLines, (uint)xcout("SKIPPED_EXE %s (MASKED)", objfile));
			goto endfunc;
		}

		execute_x(xcout("2> " CLSTDERR " > " CLSTDOUT " CL " CLOPTIONS " %s @%s", source, response));

		BuiltExeCount++;
		addElement(BuiltLines, (uint)xcout("BUILT_EXE %s", exefile));
	}
	// Disp and Make report
	{
		autoList_t *lines = readLines(CLSTDERR);
		char *line;
		uint index;

		addLines_x(lines, readLines(CLSTDOUT));

		foreach (lines, line, index)
		{
			cout("%s\n", line);
		}
		writeLines(report, lines);
		releaseDim(lines, 1);

		removeFile(CLSTDERR);
		removeFile(CLSTDOUT);
	}
	if (lastSystemRet == 0) // ? Successful
	{
		if (objmode)
		{
			// noop
		}
		else
		{
			removeFile(objfile);
			removeFile(response);
		}
		removeFile(report);
	}
	else // ? Error
	{
		remove(objfile);
		remove(exefile);

		cout("%s\\%s", c_getCwd(), source);

		writeOneLine_cx(FOUNDLISTFILE, xcout("%s\\%s", c_getCwd(), source));

		if (remCount)
			cout(" (remaining %u sources)", remCount);

		cout("\n");
		termination(1);
	}

endfunc:
	memFree(source);
	memFree(header);
	memFree(objfile);
	memFree(exefile);
	memFree(report);
	memFree(response);
	memFree(solution);
	memFree(exemanifest);
}
static void BeforeDeepBuild(int shallowMode)
{
#define PROG_NAME "BeforeBuild"

	cout("BeforeDeepBuild start\n");

	if (shallowMode)
	{
		coExecute("runsub.exe /-S " PROG_NAME);
		coExecute("runsub.exe /-S _" PROG_NAME);
	}
	else
	{
		coExecute("runsub.exe " PROG_NAME);
		coExecute("runsub.exe _" PROG_NAME);
	}
	cout("BeforeDeepBuild ended\n");

#undef PROG_NAME
}
static void AfterDeepBuild(int shallowMode)
{
#define PROG_NAME "Built"

	cout("AfterDeepBuild start\n");

	if (shallowMode)
	{
		coExecute("runsub.exe /-S " PROG_NAME);
		coExecute("runsub.exe /-S _" PROG_NAME);
	}
	else
	{
		coExecute("runsub.exe " PROG_NAME);
		coExecute("runsub.exe _" PROG_NAME);
	}
	cout("AfterDeepBuild ended\n");

#undef PROG_NAME
}
static void DeepBuild(int shallowMode)
{
	autoList_t *files;
	autoList_t *objSources;
	autoList_t *exeSources;
	char *file;
	uint index;
	uint beginRemCount = hasArgs(1) ? toValue(nextArg()) : UINTMAX;

#if 0 // 廃止 @ 2017.12.26 // 復活 @ 2020.8.20 // 廃止 @ 2022.1.22
	// 作業環境の配下であるかチェック
	{
		char *curdirY = addChar(getCwd(), '\\');

		errorCase_m(
			!lineExp("C:\\<1,19><0,2,09>\\<>", curdirY) &&
			!mbsStartsWithICase(curdirY, "C:\\Dev\\") &&
			!mbsStartsWithICase(curdirY, "C:\\Factory\\"),
			"作業環境の配下ではありません。"
			);

		memFree(curdirY);
	}
#endif

	files = ( shallowMode ? lsFiles : lssFiles )(".");

	objSources = newList();
	exeSources = newList();

	foreach (files, file, index)
	{
		if (!_stricmp("c", getExt(file)))
//		if (!_stricmp("c", getExt(file)) || !_stricmp("cpp", getExt(file)))
		{
			char *header = changeExt(file, "h");

			if (existFile(header))
			{
				addElement(objSources, (uint)strx(file));
			}
			else
			{
				addElement(exeSources, (uint)strx(file));
			}
			memFree(header);
		}
		else if (!_stricmp("sln", getExt(file)))
		{
			addElement(exeSources, (uint)strx(file));
		}
	}
	releaseDim(files, 1);

	sortJLinesICase(objSources);
	sortJLinesICase(exeSources);

	SolutionOrder(exeSources);

	files = objSources;
	addElements(files, exeSources);

	releaseAutoList(exeSources);

	if (OptimizeLevel != CLEANING_MODE)
	{
		BeforeDeepBuild(shallowMode);
	}
	foreach (files, file, index)
	{
		uint remCount = getCount(files) - index;

		cout("[%u] %s\n", remCount, file);

		if (remCount <= beginRemCount)
		{
			char *dir = getParent(file);

			if (remCount == beginRemCount)
			{
				AddExitMessage_x(xcout("began: %s", file));
				coSleep(2000);
			}
			file = getLocal(file);
			addCwd(dir);

			Build(file, remCount);

			unaddCwd();
			memFree(dir);
		}
	}
	if (OptimizeLevel != CLEANING_MODE)
	{
		AfterDeepBuild(shallowMode);
	}
	releaseDim(files, 1);

	cout("+------------+\n");
	cout("| SUCCESSFUL |\n");
	cout("+------------+\n");
	cout("BUILT_OBJ: %u (SKIPPED: %u)\n", BuiltObjCount, SkippedObjCount);
	cout("BUILT_EXE: %u (SKIPPED: %u)\n", BuiltExeCount, SkippedExeCount);

	if (BuiltSlnCount)
		cout("BUILT_SLN: %u\n", BuiltSlnCount);

	writeLines("C:\\Factory\\tmp\\Built.txt", BuiltLines);
}

int main(int argc, char **argv)
{
	BuiltLines = newList();

readArgs:
	if (argIs("--"))
	{
		OptimizeLevel = OPTLV_LINK_TEST_MODULE;
		goto readArgs;
	}
	if (argIs("clean"))
	{
		OptimizeLevel = CLEANING_MODE;
		goto readArgs;
	}
	if (argIs("++"))
	{
		ForceBuildMode = 1;
		goto readArgs;
	}

	if (argIs("***"))
	{
		ForceBuildMode = 1;
		DeepBuild(0);
	}
	else if (argIs("**"))
	{
		DeepBuild(0);
	}
	else if (argIs("*"))
	{
		DeepBuild(1);
	}
	else
	{
		Build(nextArg(), 0);
	}
	termination(0);
}
