/*
	SyncDevRange.exe [/D ルートDIR]... [/E 拡張子リスト]

		拡張子リスト ... 拡張子を '.' 区切りで指定する。例 "js", "js.jsp", "js.jsp.java"

	----

	以下のように中身が空の range は、そうでない最新の range によって上書きされる。

--		// sync > @ xxx

--		// < sync

	- - -

	★空かどうかの判定 -> IsRangeEmptyText()

	----

	更新によって using の構成が変わった場合は SyncDevUsing を使うこと。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

#define CHR_ESC_RANGE_INDENT '\x01'

#define DEF_ROOT_DIR_01 "C:\\Dev"
#define DEF_ROOT_DIR_02 "C:\\Factory"

static autoList_t *RootDirs;
static char *S_TargetExts = "c.h.cpp.cs";
static autoList_t *TargetExts;

typedef struct Range_st
{
	char *File;
	uint64 Stamp;
	char *Name;
	uint IndentLen;
	autoList_t *Lines;
	char *Text; // { 行1 + LF + 行2 + LF + ... + 行(n-1) + LF + 行n + LF }
	char *TextMD5;
	uint StartSymLineIndex;
	uint EndSymLineIndex;
}
Range_t;

static autoList_t *Ranges;

// ---- Search ----

static uint LastRangeIndentLen;
static char *LastRangeName;

static int GetLineKind(char *line) // ret: "SE-"
{
	if (lineExpICase("<\1 >////<\1 >sync<\1 >><>", line)) // ? StartSymLine らしい
	{
		errorCase(!lineExp("<\t\t>//// sync > @ <1,,__09AZaz>", line)); // ? StartSymLine ではない。

		LastRangeIndentLen = (uint)strchr(line, '/') - (uint)line;
		memFree(LastRangeName);
		LastRangeName = strx(strchr(line, '@') + 2);

		return 'S';
	}
	if (lineExpICase("<\1 >////<\1 >/<<\1 >sync<>", line)) // ? EndSymLine らしい
	{
		errorCase(!lineExp("<\t\t>//// /< sync", line)); // ? EndSymLine ではない。

		return 'E';
	}
	return '-';
}
static char *LinesToText(autoList_t *lines)
{
	char *text = strx("");
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		text = addLine(text, line);
		text = addChar(text, '\n');
	}
	return text;
}
static int IsFairRangeLine(char *line)
{
	return !strchr(line, CHR_ESC_RANGE_INDENT);
}
static char *EscapeIndentRangeLine(char *line, uint indentLen) // ret: strx()
{
	uint index;

	for (index = 0; index < indentLen; index++)
		if (line[index] != '\t')
			break;

	if (index == indentLen)
		line = xcout("%c%s", CHR_ESC_RANGE_INDENT, line + indentLen);
	else
		line = strx(line);

	return line;
}
static void Search_File(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;
	Range_t *range = NULL;

	foreach (lines, line, index)
	{
		int kind = GetLineKind(line);

		if (kind == 'S')
		{
			errorCase(range);

			range = nb_(Range_t);
			range->IndentLen = LastRangeIndentLen;
			range->Lines = newList();
			range->StartSymLineIndex = index;
		}
		else if (kind == 'E')
		{
			errorCase(!range);

			range->File = strx(file);
			getFileStamp(file, NULL, NULL, &range->Stamp);
			range->Name = strx(LastRangeName);
			range->Text = LinesToText(range->Lines);
			range->TextMD5 = md5_makeHexHashLine(range->Text);
			range->EndSymLineIndex = index;

			addElement(Ranges, (uint)range);

			range = NULL;
		}
		else if (range)
		{
			errorCase(!IsFairRangeLine(line));

			addElement(range->Lines, (uint)EscapeIndentRangeLine(line, range->IndentLen));
		}
	}
	errorCase(range);

	releaseDim(lines, 1);
}
static void Search(void)
{
	autoList_t *files = newList();
	char *dir;
	char *file;
	uint index;

	foreach (RootDirs, dir, index)
		addElements_x(files, lssFiles(dir));

	sortJLinesICase(files);

	foreach (files, file, index)
	if (findLineCase(TargetExts, getExt(file), 1) < getCount(TargetExts))
	{
		cout("file: %s\n", file);

		Search_File(file);
	}
	releaseDim(files, 1);
}

// ---- Confirm ----

static autoList_t *NeedSyncRangeNames;

static void DispAllRange_GroupByName(void)
{
	autoList_t *names = newList();
	Range_t *range;
	char *name;
	uint index;
	uint idx;

	if (!getCount(Ranges))
	{
		cout("+--------------------------------------+\n");
		cout("| // sync > ～ // sync < が１つも無い。|\n");
		cout("+--------------------------------------+\n");

		termination(0);
	}
//	errorCase_m(!getCount(Ranges), "// sync > ～ // sync < が１つも無い。"); // old

	foreach (Ranges, range, index)
		if (findLine(names, range->Name) == getCount(names))
			addElement(names, (uint)range->Name);

	rapidSortLines(names);

	cout("====\n");

	foreach (names, name, index)
	{
		uint count = 0;

		foreach (Ranges, range, idx)
		if (!strcmp(range->Name, name))
		{
			count++;
		}

		cout("%08u %s\n", count, name);
	}
	cout("====\n");

	foreach (names, name, index)
	{
		autoList_t *md5s = newList();
		int needSync;

		foreach (Ranges, range, idx)
		if (!strcmp(range->Name, name))
		{
			if (findLine(md5s, range->TextMD5) == getCount(md5s))
				addElement(md5s, (uint)range->TextMD5);
		}

		needSync = getCount(md5s) != 1;

		cout("%s %u %s\n", needSync ? "■要同期" : "□同期済", getCount(md5s), name);

		if (needSync)
			addElement(NeedSyncRangeNames, (uint)name);

#if 0 // test
		if (needSync)
		{
			addCwd("C:\\temp");
			{
				createDir(name);

				addCwd(name);
				{
					foreach (Ranges, range, idx)
					if (!strcmp(range->Name, name))
					{
						writeOneLineNoRet(range->TextMD5, range->Text);
					}
				}
				unaddCwd();
			}
			unaddCwd();
		}
#endif

		releaseAutoList(md5s);
	}
	cout("====\n");

	releaseAutoList(names);
}
static void Confirm(void)
{
	Range_t *range;
	uint index;

	foreach (Ranges, range, index)
	{
		cout("----\n");

		cout("File : %s\n", range->File);
		cout("Stamp: %I64u\n", range->Stamp);
		cout("Name : %s\n", range->Name);
		cout("T-MD5: %s\n", range->TextMD5);
		cout("Start: %u\n", range->StartSymLineIndex);
		cout("End  : %u\n", range->EndSymLineIndex);

		cout("----\n");
	}

	foreach (Ranges, range, index)
		cout("%s %s\n", range->TextMD5, range->Name);

	DispAllRange_GroupByName();

	cout("続行？\n");

	if (clearGetKey() == 0x1b)
		termination(0);

	cout("続行します。\n");
}

// ---- SyncRangeGroup ----

static void SRG_AddIndent(autoBlock_t *buff, uint indentLen)
{
	uint index;

	for (index = 0; index < indentLen; index++)
		addByte(buff, '\t');
}
static char *SRG_UnescapeIndentRangeText(char *text, uint indentLen)
{
	autoBlock_t *buff = newBlock();
	char *p;

	for (p = text; *p; p++)
	{
		if (*p == CHR_ESC_RANGE_INDENT)
			SRG_AddIndent(buff, indentLen);
		else
			addByte(buff, *p);
	}
	return unbindBlock2Line(buff);
}
static void SRG_SyncFile(Range_t *masterRange, Range_t *targetRange)
{
	autoList_t *lines = readLines(targetRange->File);
	uint index;
	FILE *fp;

	LOGPOS();

	semiRemovePath(targetRange->File);

	fp = fileOpen(targetRange->File, "wt");

	for (index = 0; index <= targetRange->StartSymLineIndex; index++)
		writeLine(fp, getLine(lines, index));

	{
		char *wkText = masterRange->Text;

		wkText = SRG_UnescapeIndentRangeText(wkText, targetRange->IndentLen);

		writeToken_x(fp, wkText);
	}

	for (index = targetRange->EndSymLineIndex; index < getCount(lines); index++)
		writeLine(fp, getLine(lines, index));

	fileClose(fp);

	setFileStamp(targetRange->File, 0, 0, targetRange->Stamp); // 更新日時を元に戻す。<- 連続して SyncDevUsing したとき、このファイルが最新にならないように。

	LOGPOS();
}
static sint Comp_RangeSSLIDesc(uint v1, uint v2)
{
	Range_t *a = (Range_t *)v1;
	Range_t *b = (Range_t *)v2;
	sint ret;

	ret = m_simpleComp(a->StartSymLineIndex, b->StartSymLineIndex) * -1;

	if (!ret)
		ret = strcmp(a->File, b->File);

	return ret;
}
static void SyncRangeGroup(autoList_t *rangeGroup)
{
	Range_t *masterRange = (Range_t *)desertElement(rangeGroup, 0);
	Range_t *targetRange;
	uint index;

	rapidSort(rangeGroup, Comp_RangeSSLIDesc);

	// 更新順確認
	{
		cout("---- 更新順確認 ----\n");

		{
			Range_t *range;
			uint range_index;

			cout("< %s (%u)\n", masterRange->File, masterRange->StartSymLineIndex);

			foreach (rangeGroup, range, range_index)
			{
				cout("> %s (%u)\n", range->File, range->StartSymLineIndex);
			}
		}

		cout("続行？\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("続行します。\n");
	}

	foreach (rangeGroup, targetRange, index)
	{
		if (!strcmp(masterRange->TextMD5, targetRange->TextMD5)) // ? 同じ -> 更新不要
		{
			LOGPOS();
		}
		else
		{
			SRG_SyncFile(masterRange, targetRange);
		}
	}
}

// ---- ProcAllRange ----

static int IsRangeEmptyText(Range_t *range)
{
	char *text = strx(range->Text);
	int ret;

	ucTrim(text);

	ret = text[0] == '\0';

	memFree(text);
	return ret;
}
static sint Comp_RangeTextMD5(uint v1, uint v2)
{
	Range_t *a = (Range_t *)v1;
	Range_t *b = (Range_t *)v2;
	sint ret;

	ret = strcmp(a->TextMD5, b->TextMD5);

	if (!ret)
		ret = strcmp(a->File, b->File);

	return ret;
}
static void DispRangeGroup(autoList_t *rangeGroup)
{
	Range_t *range;
	uint index;

	foreach (rangeGroup, range, index)
	{
		cout("----\n");

		cout("File : %s\n", range->File);
		cout("T-MD5: %s\n", range->TextMD5);
		cout("Start: %u\n", range->StartSymLineIndex);

		cout("----\n");
	}

	foreach (rangeGroup, range, index)
	{
		cout("%s (%u)\n", range->File, range->StartSymLineIndex);
	}
	cout("====\n");

	foreach (rangeGroup, range, index)
	{
		cout("%s %c %I64u %u\n", range->TextMD5, IsRangeEmptyText(range) ? '-' : '+', range->Stamp, strlen(range->Text));
	}
	cout("====\n");
	cout("Represents:\n");

	{
		autoList_t *tmp_rangeGroup = copyAutoList(rangeGroup);
		FILE *foundList_fp = fileOpen(FOUNDLISTFILE, "wt"); // ついでに FOUNDLISTFILE へ出力

		rapidSort(tmp_rangeGroup, Comp_RangeTextMD5);

		foreach (tmp_rangeGroup, range, index)
		if (!index || strcmp(range->TextMD5, ((Range_t *)getElement(tmp_rangeGroup, index - 1))->TextMD5))
		{
			cout("%s %s\n", range->TextMD5, range->File);
			writeLine(foundList_fp, range->File);
		}
		releaseAutoList(tmp_rangeGroup);
		fileClose(foundList_fp);
	}

	cout("====\n");
}
static sint Comp_RangeStampDesc(uint v1, uint v2)
{
	Range_t *a = (Range_t *)v1;
	Range_t *b = (Range_t *)v2;
	sint ret;

	// 中身あり -> 中身空っぽ　の順
	{
		sint et1 = IsRangeEmptyText(a) ? 1 : 0;
		sint et2 = IsRangeEmptyText(b) ? 1 : 0;

		ret = et1 - et2;
		if (ret)
			return ret;
	}

	ret = m_simpleComp(a->Stamp, b->Stamp) * -1;
	if (ret)
		return ret;

	ret = strcmp(a->File, b->File);
	if (ret)
		return ret;

	ret = m_simpleComp(a->StartSymLineIndex, b->StartSymLineIndex);
	return ret;
}
static int IsRangeGroupExpectedCond_Case1(autoList_t *rangeGroup) // 1ヶ所だけ修正した場合
{
	uint index;

	for (index = 1; index < getCount(rangeGroup); index++)
	{
		Range_t *r1 = (Range_t *)getElement(rangeGroup, index - 1);
		Range_t *r2 = (Range_t *)getElement(rangeGroup, index - 0);

		if (index == 1)
		{
			if (!strcmp(r1->TextMD5, r2->TextMD5)) // 最初と2番目は異なるはず。
				return 0;
		}
		else
		{
			if (strcmp(r1->TextMD5, r2->TextMD5)) // それ以降は同じはず。
				return 0;
		}
	}
	return 1;
}
static int IsRangeGroupExpectedCond_Case2(autoList_t *rangeGroup) // 1ヶ所以上、空の range を追加した場合
{
	uint index;

	for (index = 1; index < getCount(rangeGroup); index++)
	{
		Range_t *r1 = (Range_t *)getElement(rangeGroup, index - 1);
		Range_t *r2 = (Range_t *)getElement(rangeGroup, index - 0);

		if (strcmp(r1->TextMD5, r2->TextMD5)) // 基本的に同じはず。
		{
			if (!IsRangeEmptyText(r2)) // 異なる場合は、少なくとも後側が空っぽのはず。
			{
				return 0;
			}
		}
	}
	return 1;
}
static int IsRangeGroupExpectedCond(autoList_t *rangeGroup)
{
	return
		IsRangeGroupExpectedCond_Case1(rangeGroup) ||
		IsRangeGroupExpectedCond_Case2(rangeGroup);
}
static void ProcRangeGroup(autoList_t *rangeGroup)
{
	rapidSort(rangeGroup, Comp_RangeStampDesc);

	DispRangeGroup(rangeGroup);

	if (!IsRangeGroupExpectedCond(rangeGroup))
	{
		cout("★★★想定外の修正状態です。続行？\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("続行します。\n");
	}
	SyncRangeGroup(rangeGroup);
}
static void ProcAllRange(void)
{
	autoList_t *rangeGroup = newList();
	char *name;
	uint index;

	foreach (NeedSyncRangeNames, name, index)
	{
		Range_t *range;
		uint range_index;

		setCount(rangeGroup, 0);

		foreach (Ranges, range, range_index)
			if (!strcmp(range->Name, name))
				addElement(rangeGroup, (uint)range);

		ProcRangeGroup(rangeGroup);
	}
	releaseAutoList(rangeGroup);
}

// ----

int main(int argc, char **argv)
{
	char *dir;
	uint index;

	RootDirs = newList();
	Ranges = newList();
	NeedSyncRangeNames = newList();

readArgs:
	if (argIs("/D"))
	{
		addElement(RootDirs, (uint)nextArg());
		goto readArgs;
	}
	if (argIs("/E"))
	{
		S_TargetExts = nextArg();
		goto readArgs;
	}
	errorCase(hasArgs(1)); // 不明なコマンド引数

	if (!getCount(RootDirs))
	{
		addElement(RootDirs, (uint)DEF_ROOT_DIR_01);
		addElement(RootDirs, (uint)DEF_ROOT_DIR_02);
	}
	foreach (RootDirs, dir, index)
		errorCase(!existDir(dir));

	TargetExts = tokenize(S_TargetExts, '.');
	trimLines(TargetExts);

	errorCase(!getCount(TargetExts));

	Search();
	Confirm();
	ProcAllRange();

	cout("\\e\n");
}
