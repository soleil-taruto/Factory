/*
	CopyLib.exe 入力DIR 出力DIR

		入力DIR, 出力DIR ... 存在するディレクトリであること。

	環境変数

		@CopyLib_NoPause に 非0 ... 終了時ポーズしない。
*/

#include "C:\Factory\Common\all.h"

static int IsNoPause(void)
{
	return toValue(getEnvLine("@CopyLib_NoPause")) != 0;
}
static int IsCSharpFile(char *file)
{
	return !_stricmp("cs", getExt(file));
}

// ---- Dis-Comment (discmt) Entered ----

static int DCE_DisCmtEntered;

static void DCE_Start(void)
{
	errorCase(DCE_DisCmtEntered);
}
static void DCE_End(void)
{
	errorCase(DCE_DisCmtEntered);
}
static void DCE_SetLine(char *line)
{
	int enter;
	int leave;

	{
		char *tmp = strx(line);

		ucTrim(tmp);

		enter = startsWith(tmp, "// discmt >");
		leave = startsWith(tmp, "// < discmt");

		memFree(tmp);
	}

	if (enter)
	{
		errorCase(DCE_DisCmtEntered);
		DCE_DisCmtEntered = 1;
	}
	else if (leave)
	{
		errorCase(!DCE_DisCmtEntered);
		DCE_DisCmtEntered = 0;
	}
}
static int DCE_IsOutDisCmt(void)
{
	return !DCE_DisCmtEntered;
}

// ----

static autoList_t *ResAutoComment;
static autoList_t *ResAutoComment_CS;

static void AutoComment(autoList_t *ranges)
{
	autoList_t *range;
	uint range_index;
	char *line;
	uint index;
	int commentEntered = 0;
	int classEntered = 0;

	DCE_Start();

	foreach (ranges, range, range_index)
	foreach (range, line, index)
	{
		int insCmt;

		DCE_SetLine(line);

		line = strx(line);
		nn_strstr(line, "//")[0] = '\0'; // インラインコメントの除去
		trimTrail(line, ' ');

		if (startsWith(line, "class "))
			classEntered = 1;

		if (!strcmp(line, "};"))
			classEntered = 0;

		if (!strcmp(line, "}"))
			classEntered = 0;

		// set insCmt >

		if (classEntered)
		{
			insCmt = line[0] == '\t' && (m_isalpha(line[1]) || line[1] == '_' || line[1] == '~');

			if (startsWith(line, "#define "))
				insCmt = 1;

			if (index && !strcmp(getLine(range, index - 1), "\t*/"))
				insCmt = 0;

			if (index && startsWith(getLine(range, index - 1), "#define "))
				insCmt = 0;

			if (!strcmp(line, "\t/*"))
				insCmt = 1;
		}
		else
		{
			insCmt = m_isalpha(line[0]) || line[0] == '_';

			if (startsWith(line, "#define "))
				insCmt = 1;

			if (index && !strcmp(getLine(range, index - 1), "}") && endsWith(line, ";")) // typedef など
				insCmt = 0;

			if (index && !strcmp(getLine(range, index - 1), "*/"))
				insCmt = 0;

			if (index && startsWith(getLine(range, index - 1), "template <"))
				insCmt = 0;

			if (!strcmp(line, "/*"))
				insCmt = 1;
		}

		if (endsWith(line, ":"))
			insCmt = 0;

		if (commentEntered)
			insCmt = 0;

		if (!range_index && !index)
			insCmt = 1;

		// < set insCmt

		if (insCmt && DCE_IsOutDisCmt() && range_index % 2 != 1) // ? .. && .. && アプリ固有コード以外
		{
			char *comment;
			uint comment_index;

			foreach (ResAutoComment, comment, comment_index)
			{
				insertElement(range, index++, (uint)xcout("%s%s", classEntered ? "\t" : "", comment));
			}
		}

		{
			char *tmp = strx(getLine(range, index));
//			char *tmp = strx(line); // line はインラインコメントが削除されている。

			ucTrim(tmp);

			if (commentEntered)
			{
				if (!strcmp(tmp, "*/"))
					commentEntered = 0;

				if (!strcmp(tmp, "//*/")) // LOG_ENABLED んとことかの /* ～ /*/ ～ //*/
					commentEntered = 0;
			}
			else
			{
				if (!strcmp(tmp, "/*"))
					commentEntered = 1;
			}
			memFree(tmp);
		}

		memFree(line);
	}
	errorCase(commentEntered);
	errorCase(classEntered);

	DCE_End();
}
static void AutoComment_CS(autoList_t *ranges)
{
	autoList_t *range;
	uint range_index;
	char *line;
	uint index;

	DCE_Start();

	foreach (ranges, range, range_index)
	foreach (range, line, index)
	{
		char *prevLine = index ? getLine(range, index - 1) : "";
		char *insCmtIndent = NULL;

		DCE_SetLine(line);

		if (!startsWith(prevLine, "\t/// "))
		if (
			startsWith(line, "\t/// ") ||
			startsWith(line, "\tpublic class ") ||
			startsWith(line, "\tpublic static class ") ||
			startsWith(line, "\tpublic interface ")
			)
			insCmtIndent = "\t";

		if (!startsWith(prevLine, "\t\t/// "))
		if (!startsWith(prevLine, "\t\t[DllImport"))
		if (!startsWith(prevLine, "\t\t[StructLayout"))
		if (
			startsWith(line, "\t\t/// ") ||
			startsWith(line, "\t\t[DllImport") ||
			startsWith(line, "\t\t[StructLayout") ||
			startsWith(line, "\t\tpublic ") ||
			startsWith(line, "\t\tprivate ") ||
			startsWith(line, "\t\tprotected ")
			)
			insCmtIndent = "\t\t";

		if (insCmtIndent && DCE_IsOutDisCmt() && range_index % 2 != 1) // ? .. && .. && アプリ固有コード以外
		{
			char *comment;
			uint comment_index;

			foreach (ResAutoComment_CS, comment, comment_index)
			{
				insertElement(range, index++, (uint)xcout("%s%s", insCmtIndent, comment));
			}
		}

		// TODO @" など、いずれ対応が必要になると思う。
	}

	DCE_End();
}
static autoList_t *ReadCommonAndAppSpecRanges(char *file)
{
	autoList_t *ranges = newList();
	autoList_t *lines = newList();
	FILE *fp = fileOpen(file, "rt");
	char *line;
	int appSpecEntered = 0;

	while (line = readLine(fp))
	{
		int enter;
		int leave;

		{
			char *tmp = strx(line);

			ucTrim(tmp);

			enter = startsWith(tmp, "// app >");
			leave = startsWith(tmp, "// < app");

			memFree(tmp);
		}

		errorCase(enter && leave);

		if (leave)
		{
			errorCase(!appSpecEntered);
			appSpecEntered = 0;

			addElement(ranges, (uint)lines);
			lines = newList();
		}
		addElement(lines, (uint)line);

		if (enter)
		{
			errorCase(appSpecEntered);
			appSpecEntered = 1;

			addElement(ranges, (uint)lines);
			lines = newList();
		}
	}
	errorCase(appSpecEntered);

	addElement(ranges, (uint)lines);

	fileClose(fp);
	return ranges;
}
static int IsEmptyRange(autoList_t *ranges, uint rangeIndex)
{
	autoList_t *range;
	char *line;
	uint index;

	errorCase(getCount(ranges) % 2 != 1);
	errorCase(rangeIndex % 2 != 1);
	errorCase(getCount(ranges) <= rangeIndex);

	LOGPOS();

	range = getList(ranges, rangeIndex);

	foreach (range, line, index)
	{
		if (!lineExp("<\t\t  >", line)) // ? ! (空行 || 空白のみの行)
		{
			cout("コードの記述有り\n");
			return 0;
		}
	}
	cout("コードの記述無し\n");
	return 1;
}
static void WeldRange(autoList_t *ranges, uint rangeIndex)
{
	errorCase(getCount(ranges) % 2 != 1);
	errorCase(rangeIndex % 2 != 1);
	errorCase(getCount(ranges) <= rangeIndex);

	LOGPOS();

	addElements(getList(ranges, rangeIndex - 1), getList(ranges, rangeIndex + 0));
	addElements(getList(ranges, rangeIndex - 1), getList(ranges, rangeIndex + 1));

	desertElement(ranges, rangeIndex);
	desertElement(ranges, rangeIndex);

	LOGPOS();
}
static void WeldAllEmptyRange(autoList_t *ranges)
{
	uint index;

	LOGPOS();

	errorCase(getCount(ranges) % 2 != 1); // 2bs

	for (index = 2; index < getCount(ranges); )
	{
		char *h = (char *)getLastElement(getList(ranges, index - 2));

		coutJLine_x(xcout("h: %s", h));

		if (IsEmptyRange(ranges, index - 1))
		{
			cout("コードの記述無し -> 削除\n");

			WeldRange(ranges, index - 1);
		}
		else
		{
			cout("コードの記述有り -> 何もしない\n");

			index += 2;
		}
	}
	LOGPOS();
}
static void WeldAllNewlyAddedRange(autoList_t *rRanges, autoList_t *wRanges)
{
	uint index;

	LOGPOS();

	errorCase(getCount(rRanges) % 2 != 1); // 2bs
	errorCase(getCount(wRanges) % 2 != 1); // 2bs

	for (index = 2; index < getCount(rRanges); )
	{
		cout("index: %u\n", index);

		if (index < getCount(wRanges))
		{
			char *r = (char *)getLastElement(getList(rRanges, index - 2));
			char *w = (char *)getLastElement(getList(wRanges, index - 2));

			coutJLine_x(xcout("r: %s", r));
			coutJLine_x(xcout("w: %s", w));

			if (!strcmp(r, w))
			{
				cout("同じアプリ固有コード\n");

				index += 2;
			}
			else
			{
				cout("違うアプリ固有コード -> 新しく追加された可能性\n");
				cout("削除して続行\n");

				WeldRange(rRanges, index - 1);
			}
		}
		else
		{
			char *r = (char *)getLastElement(getList(rRanges, index - 2));

			coutJLine_x(xcout("r: %s", r));
			cout("w-\n");

			cout("多すぎるアプリ固有コード -> 新しく追加された可能性\n");
			cout("削除して続行\n");

			WeldRange(rRanges, index - 1);
		}
	}
	LOGPOS();
}
static void CheckAppSpecRangesPair(autoList_t *rRanges, autoList_t *wRanges)
{
	uint index;

	LOGPOS();

	errorCase_m(getCount(rRanges) != getCount(wRanges), "アプリ固有コードの数が合わないため上書きできません。");

	errorCase(getCount(rRanges) % 2 != 1); // 2bs
	errorCase(getCount(wRanges) % 2 != 1); // 2bs

	for (index = 2; index < getCount(rRanges); index += 2)
	{
		{
			char *r = (char *)getLastElement(getList(rRanges, index - 2));
			char *w = (char *)getLastElement(getList(wRanges, index - 2));

			coutJLine_x(xcout("r: %s", r));
			coutJLine_x(xcout("w: %s", w));

			errorCase_m(strcmp(r, w), "アプリ固有コード開始行不一致");
		}

		{
			char *r = getLine(getList(rRanges, index), 0);
			char *w = getLine(getList(wRanges, index), 0);

			coutJLine_x(xcout("r: %s", r));
			coutJLine_x(xcout("w: %s", w));

			errorCase_m(strcmp(r, w), "アプリ固有コード終了行不一致");
		}
	}
	LOGPOS();
}

static int DCL_ExistNewFile;

static void DoCopyLib(char *rDir, char *wDir, int testMode)
{
	autoList_t *rSubDirs = lssDirs(rDir);
	autoList_t *wSubDirs = lssDirs(wDir);
	autoList_t *rFiles = lssFiles(rDir);
	autoList_t *wFiles = lssFiles(wDir);
	autoList_t *owFiles;
	char *dir;
	char *file;
	uint index;

	changeRoots(rSubDirs, rDir, NULL);
	changeRoots(wSubDirs, wDir, NULL);
	changeRoots(rFiles, rDir, NULL);
	changeRoots(wFiles, wDir, NULL);

	releaseDim(mergeLinesICase(rSubDirs, wSubDirs), 1); // 何もすることは無いので owDirs は不要
	reverseElements(wSubDirs); // 削除は後ろから行うため、ここで逆転しておく。

	owFiles = mergeLinesICase(rFiles, wFiles);

	DCL_ExistNewFile = 1 <= getCount(rFiles);

	foreach (rSubDirs, dir, index)
		if (!testMode)
			createPath_x(combine(wDir, dir), 'D');

	foreach (rFiles, file, index)
	{
		char *rFile = combine(rDir, file);
		char *wFile = combine(wDir, file);

		cout("R %s\n", rFile);
		cout("> %s\n", wFile);

		if (!testMode)
			copyFile(rFile, wFile);

		memFree(rFile);
		memFree(wFile);
	}
	foreach (wFiles, file, index)
	{
		int csMode = IsCSharpFile(file);
		char *wFile = combine(wDir, file);

		cout("D %s\n", wFile);

		{
			autoList_t *ranges = ReadCommonAndAppSpecRanges(wFile);

			cout("Dr.1 %u\n", getCount(ranges));
			WeldAllEmptyRange(ranges);
			cout("Dr.2 %u\n", getCount(ranges));

			errorCase_m(1 < getCount(ranges), "アプリ固有コードを含むため削除できません。\nファイルを手動で削除して下さい。");

			releaseDim(ranges, 2);
		}

		if (!testMode)
		{
			if (csMode)
				removeFile(wFile);
			else
				writeOneLine(wFile, "// deleted"); // 削除すると .vcxproj のエントリーを消せなくなるので、削除しない。
		}
		memFree(wFile);
	}
	foreach (owFiles, file, index)
	{
		int csMode = IsCSharpFile(file);
		char *rFile = combine(rDir, file);
		char *wFile = combine(wDir, file);
		autoList_t *rRanges;
		autoList_t *wRanges;

		cout("< %s\n", rFile);
		cout("> %s\n", wFile);

		rRanges = ReadCommonAndAppSpecRanges(rFile);
		wRanges = ReadCommonAndAppSpecRanges(wFile);

		(csMode ? AutoComment_CS : AutoComment)(rRanges); // rRanges が WeldRange() されるより先に実行する。

		cout("1.<r %u\n", getCount(rRanges));
		cout("1.>r %u\n", getCount(wRanges));

		WeldAllEmptyRange(wRanges);

		cout("2.<r %u\n", getCount(rRanges));
		cout("2.>r %u\n", getCount(wRanges));

		WeldAllNewlyAddedRange(rRanges, wRanges);

		cout("3.<r %u\n", getCount(rRanges));
		cout("3.>r %u\n", getCount(wRanges));

		CheckAppSpecRangesPair(rRanges, wRanges);

		{
			autoList_t *lines = newList();
			uint index;

			for (index = 0; index < getCount(rRanges); index++)
				addElements(lines, getList(index % 2 ? wRanges : rRanges, index));

			if (!testMode)
			{
//				semiRemovePath(wFile); // zantei
				writeLines(wFile, lines);
			}
			releaseAutoList(lines);
		}

		memFree(rFile);
		memFree(wFile);
		releaseDim(rRanges, 2);
		releaseDim(wRanges, 2);
	}
	foreach (wSubDirs, dir, index)
		if (!testMode)
			removeDirIfEmpty_x(combine(wDir, dir)); // .cpp の場合ファイルを削除しないのでDIRを削除できない場合もある。

	releaseDim(rSubDirs, 1);
	releaseDim(wSubDirs, 1);
	releaseDim(rFiles, 1);
	releaseDim(wFiles, 1);
	releaseDim(owFiles, 1);
}
static void CopyLib(char *rDir, char *wDir)
{
	rDir = makeFullPath(rDir);
	wDir = makeFullPath(wDir);

	cout("< %s\n", rDir);
	cout("> %s\n", wDir);

	errorCase(!existDir(rDir));
	errorCase(!existDir(wDir));
	errorCase(!_stricmp(rDir, wDir)); // ? 同じディレクトリ

	LOGPOS();
	DoCopyLib(rDir, wDir, 1);
	LOGPOS();
	DoCopyLib(rDir, wDir, 0);
	LOGPOS();

	if (DCL_ExistNewFile) // 新規追加ファイルがあった場合は２回行う必要がある。新規追加ファイルには AutoComment が適用されない。
	{
		LOGPOS();
		DoCopyLib(rDir, wDir, 1);
		LOGPOS();
		DoCopyLib(rDir, wDir, 0);
		LOGPOS();
	}
	memFree(rDir);
	memFree(wDir);
}
int main(int argc, char **argv)
{
	{
		char *cmtFile = changeExt(getSelfFile(), "txt");

		ResAutoComment = readLines(cmtFile);

		memFree(cmtFile);
	}

	{
		char *cmtFile = addLine(changeExt(getSelfFile(), ""), "_CS.txt");

		ResAutoComment_CS = readLines(cmtFile);

		memFree(cmtFile);
	}

	CopyLib(getArg(0), getArg(1));

	if (!IsNoPause())
	{
		cout("Press any key to continue ...");
		getKey();
		cout("\n");
	}
}
