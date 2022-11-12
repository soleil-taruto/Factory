/*
	SyncDevUsing.exe [/D ���[�gDIR]... [/E �g���q���X�g]

		�g���q���X�g ... �g���q�� '.' ��؂�Ŏw�肷��B�� "js", "js.jsp", "js.jsp.java"

	----

	�ȉ��̂悤�ɒ��g����̏ꍇ�̓R�s�[���ɂȂ�Ȃ��悤�ɂ����B

		�t�@�C���̐擪
		��s�܂��͋󔒂����̍s (0�`INF �s)
--		// ^ sync @ xxx

	- - -

	���󂩂ǂ����̔��� -> Eval_NotEmptyEmpty()
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

#define DEF_ROOT_DIR_01 "C:\\Dev"
#define DEF_ROOT_DIR_02 "C:\\Factory"

static autoList_t *RootDirs;
static char *S_TargetExts = "c.h.cpp.cs";
static autoList_t *TargetExts;

typedef struct Header_st
{
	char *File;
	uint64 Stamp;
	char *Name;
	autoList_t *Lines;
	char *Text; // { �s1 + LF + �s2 + LF + ... + �s(n-1) + LF + �sn + LF }
	char *TextMD5;
	uint SymLineIndex;
}
Header_t;

static autoList_t *Headers;

// ---- Search ----

static char *LastHeaderName;

static int IsSymLine(char *line)
{
	if (lineExpICase("<\1 >////<\1 >^<\1 >sync<>", line)) // ? SymLine �炵��
	{
		errorCase(!lineExp("<\t\t>//// ^ sync @ <1,,__09AZaz>", line)); // ? SymLine �ł͂Ȃ��B

		memFree(LastHeaderName);
		LastHeaderName = strx(strchr(line, '@') + 2);

		return 1;
	}
	return 0;
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
static void Search(void)
{
	autoList_t *files = newList();
	char *dir;
	char *file;
	uint index;

	foreach (RootDirs, dir, index)
		addElements_x(files, lssFiles(dir));

	foreach (files, file, index)
	if (findLineCase(TargetExts, getExt(file), 1) < getCount(TargetExts))
	{
		autoList_t *lines = readLines(file);
		char *line;
		uint line_index;
		Header_t *header = NULL;

		cout("file: %s\n", file);

		foreach (lines, line, line_index)
		{
			if (IsSymLine(line))
			{
				autoList_t gal;

				errorCase(header); // ? already exists in the file
				header = nb_(Header_t);

				header->File = strx(file);
				getFileStamp(file, NULL, NULL, &header->Stamp);
				header->Name = strx(LastHeaderName);
				header->Lines = copyLines(gndSubElementsVar(lines, 0, line_index, gal));
				header->Text = LinesToText(header->Lines);
				header->TextMD5 = md5_makeHexHashLine(header->Text);
				header->SymLineIndex = line_index;

				addElement(Headers, (uint)header);

				LOGPOS();
				break;
			}
		}
		releaseDim(lines, 1);
	}
	releaseDim(files, 1);
}

// ---- Confirm ----

static autoList_t *NeedSyncHeaderNames;

static void Confirm(void)
{
	autoList_t *names = newList();
	Header_t *header;
	char *name;
	uint index;
	uint idx;

	foreach (Headers, header, index)
		if (findLine(names, header->Name) == getCount(names))
			addElement(names, (uint)header->Name);

	rapidSortLines(names);

	foreach (names, name, index)
	{
		autoList_t *md5s = newList();
		int needSync;

		foreach (Headers, header, idx)
		if (!strcmp(header->Name, name))
		{
			if (findLine(md5s, header->TextMD5) == getCount(md5s))
				addElement(md5s, (uint)header->TextMD5);
		}

		needSync = getCount(md5s) != 1;

		cout("%s %s\n", needSync ? "��������" : "��������", name);

		if (needSync)
			addElement(NeedSyncHeaderNames, (uint)name);
	}

	cout("���s�H\n");

	if (clearGetKey() == 0x1b)
		termination(0);

	cout("���s���܂��B\n");
}

// ---- ProcAllHeaders ----

static void SH_Main(Header_t *masterHeader, Header_t *targetHeader)
{
	autoList_t *lines = readLines(targetHeader->File);
	uint index;
	FILE *fp;

	LOGPOS();

	semiRemovePath(targetHeader->File);

	fp = fileOpen(targetHeader->File, "wt");

	writeToken(fp, masterHeader->Text);

	for (index = targetHeader->SymLineIndex; index < getCount(lines); index++)
		writeLine(fp, getLine(lines, index));

	fileClose(fp);

	setFileStamp(targetHeader->File, 0, 0, targetHeader->Stamp); // �X�V���������ɖ߂��B<- �A������ SyncDevRange �����Ƃ��A���̃t�@�C�����ŐV�ɂȂ�Ȃ��悤�ɁB

	LOGPOS();
}
static void SyncHeader(Header_t *masterHeader, Header_t *targetHeader)
{
	if (!strcmp(masterHeader->TextMD5, targetHeader->TextMD5)) // ? ���� -> �X�V�s�v
	{
		LOGPOS();
	}
	else
	{
		SH_Main(masterHeader, targetHeader);
	}
}
static sint Eval_NotEmptyEmpty(Header_t *header)
{
	char *text = strx(header->Text);
	sint ret;

	if (startsWith(text, "\xEF\xBB\xBF")) // BOM
		eraseLine(text, 3);

	ucTrim(text);
	ret = *text ? 1 : 2;
	memFree(text);
	return ret;
}
static sint Comp_HeaderStampDesc(uint v1, uint v2)
{
	Header_t *a = (Header_t *)v1;
	Header_t *b = (Header_t *)v2;
	sint ret;

	ret = Eval_NotEmptyEmpty(a) - Eval_NotEmptyEmpty(b);
	if (ret)
		return ret;

	ret = m_simpleComp(a->Stamp, b->Stamp) * -1;
	if (ret)
		return ret;

	ret = strcmp(a->File, b->File);
	return ret;
}
static void ProcHeaderGroup(autoList_t *headerGroup)
{
	Header_t *header;
	uint index;

	rapidSort(headerGroup, Comp_HeaderStampDesc);

	// �X�V���m�F
	{
		cout("---- �X�V���m�F ----\n");

		foreach (headerGroup, header, index)
			cout("%c %s (%u)\n", index ? '>' : '<', header->File, header->SymLineIndex);

		cout("���s�H\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("���s���܂��B\n");
	}

	foreach (headerGroup, header, index)
	if (index)
	{
		SyncHeader((Header_t *)getElement(headerGroup, 0), header);
	}
}
static void ProcAllHeaders(void)
{
	autoList_t *headerGroup = newList();
	char *name;
	uint index;

	foreach (NeedSyncHeaderNames, name, index)
	{
		Header_t *header;
		uint header_index;

		setCount(headerGroup, 0);

		foreach (Headers, header, header_index)
			if (!strcmp(header->Name, name))
				addElement(headerGroup, (uint)header);

		ProcHeaderGroup(headerGroup);
	}
	releaseAutoList(headerGroup);
}

// ----

int main(int argc, char **argv)
{
	char *dir;
	uint index;

	RootDirs = newList();
	Headers = newList();
	NeedSyncHeaderNames = newList();

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
	errorCase(hasArgs(1)); // �s���ȃR�}���h����

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
	ProcAllHeaders();

	cout("\\e\n");
}
