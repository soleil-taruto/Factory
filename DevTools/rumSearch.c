/*
	デフォルトの検索対象 == カレントの配下にある全ての .rum ディレクトリ

	rumSearch.exe [/E SPEC-EXTS] [/I] [/T] [/R RUM-DIR]... FIND-PATTERN

		SPEC-EXTS ... 検索対象の拡張子を '.' 区切りで記述する。

			例) c.h.txt
*/

#include "C:\Factory\Common\all.h"

#define RUM_DIR_EXT "rum"
#define DIR_FILES "files"
#define DIR_REVISIONS "revisions"
#define FILE_FILES "files.txt"

static char *FindPattern; // "" 不可
static autoList_t *RumDirs;
static autoList_t *SpecExts;
static int IgnoreCase;
static int TokenOnlyMode;

static uint64 TotalCount;
static uint64 TotalFileCount;
static autoList_t *FoundRumDirs;

// ---- read stream ---

static FILE *RFp;
static uint64 RIndex;

static int ReadChar(uint64 index)
{
	int chr;

	if (index != RIndex)
	{
		fileSeek(RFp, SEEK_SET, index);
		RIndex = index;
	}
	chr = readChar(RFp);
	RIndex++;
	return chr;
}

// ----

static void DispRange(sint64 start, sint64 end, sint64 fileSize)
{
	sint64 index;
	int chr;

	for (index = start; index <= end; index++)
	{
		if (0 <= index && index < fileSize)
		{
			chr = ReadChar((uint64)index);

			if (0x20 <= chr && chr <= 0x7e || 0xa1 <= chr && chr <= 0xdf)
			{
				// noop
			}
			else
			{
				chr = 0xc0 | chr & 0x1f;
			}
		}
		else
		{
			chr = '?'; // ファイルの先頭より前 || ファイルの終端より後
		}
		cout("%c", chr);
	}
}
static void SearchEntFile(char *entFile, char *file, char *revision, char *rumDir)
{
	uint fndPtnSize;
	uint64 fileSize;
	uint64 linecnt;
	uint64 fndcnt;
	uint64 index;
	uint matchcnt;
	int chr;

	if (SpecExts)
	{
		char *fileExt = getExt(file);
		char *ext;
		uint index;

		foreach (SpecExts, ext, index)
			if (!_stricmp(ext, fileExt))
				break;

		if (!ext) // ? SpecExts に一致する拡張子が無い -> 検索対象外
			return;
	}
	fndPtnSize = strlen(FindPattern);
	fileSize = getFileSize(entFile);
	linecnt = 0;
	fndcnt = 0;
	RFp = fileOpen(entFile, "rb");
	RIndex = 0;

	for (index = 0; index + fndPtnSize <= fileSize; )
	{
		int newLineFlag = 0;

		for (matchcnt = 0; matchcnt < fndPtnSize; matchcnt++)
		{
			int chr = ReadChar(index + matchcnt);

			if (!matchcnt && chr == '\n')
				newLineFlag = 1;

			if (IgnoreCase)
				chr = m_tolower(chr);

			if (chr != FindPattern[matchcnt])
				break;
		}
		if (matchcnt == fndPtnSize) // ? 一致した。
		{
			if (TokenOnlyMode)
			{
				if (0 < index) // ? 一致した場所がファイルの先頭ではない。
				{
					chr = ReadChar(index - 1);

					if ('\x20' < chr)
						goto notMatch;
				}
				if (index + fndPtnSize < fileSize) // ? 一致した場所がファイルの終端ではない。
				{
					chr = ReadChar(index + fndPtnSize);

					if ('\x20' < chr)
						goto notMatch;
				}
			}

			// Found

			if (!fndcnt)
			{
				TotalFileCount++;

				if (findElement(FoundRumDirs, (uint)rumDir, simpleComp) == getCount(FoundRumDirs))
					addElement(FoundRumDirs, (uint)rumDir);

				cout("%s\n", rumDir);
				cout("[%s]\n", revision);
				cout("%s\n", file);
			}
			TotalCount++;
			fndcnt++;
			index += fndPtnSize;

			cout("%9I64u %9I64u ", fndcnt, linecnt);

			{
			sint64 ndx1b;
			sint64 ndx1e;
			sint64 ndx2b;
			sint64 ndx2e;

			ndx2b = index;
			ndx2e = ndx2b + 28;
			ndx1e = ndx2b - fndPtnSize - 1;
			ndx1b = ndx1e - 28;

			DispRange(ndx1b, ndx1e, (sint64)fileSize);
			cout(" ");
			DispRange(ndx2b, ndx2e, (sint64)fileSize);
			cout("\n");
			}
		}
		else
		{
		notMatch:
			index++;
		}
		if (newLineFlag)
			linecnt++;
	}
	fileClose(RFp);
	RFp = NULL;
	RIndex = 0;
}
static void SearchRumDir(char *rumDir)
{
	char *filesDir = combine(rumDir, DIR_FILES);
	char *revisionsDir = combine(rumDir, DIR_REVISIONS);
	autoList_t *dirs;
	char *dir;
	uint index;

	dirs = lsDirs(revisionsDir);
	rapidSortLines(dirs);

	foreach (dirs, dir, index)
	{
		char *filesFile = combine(dir, FILE_FILES);
		char *revision = getLocal(dir);
		autoList_t *lines;
		char *line;
		uint line_index;

		lines = readLines(filesFile);

		foreach (lines, line, line_index)
		{
			char *hash;
			char *file;
			char *entFile;

			errorCase(!lineExp("<32,09AFaf> <>", line));

			line[32] = '\0';
			hash = line;
			file = line + 33;
			entFile = combine(filesDir, hash);

			SearchEntFile(entFile, file, revision, rumDir);

			memFree(entFile);
		}
		releaseDim(lines, 1);
		memFree(filesFile);
	}
	releaseDim(dirs, 1);
	memFree(revisionsDir);
	memFree(filesDir);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/E"))
	{
		SpecExts = tokenize(nextArg(), '.');
		goto readArgs;
	}
	if (argIs("/I"))
	{
		IgnoreCase = 1;
		goto readArgs;
	}
	if (argIs("/T"))
	{
		TokenOnlyMode = 1;
		goto readArgs;
	}
	if (argIs("/R"))
	{
		char *dir = nextArg();

		if (!RumDirs)
			RumDirs = newList();

		dir = makeFullPath(dir);

		errorCase(!existDir(dir));
		errorCase(_stricmp(RUM_DIR_EXT, getExt(dir)));

		addElement(RumDirs, (uint)dir);
		goto readArgs;
	}

	FindPattern = nextArg();
	errorCase(!*FindPattern);

	if (IgnoreCase)
		toLowerLine(FindPattern);

	if (!RumDirs)
	{
		autoList_t *dirs = lssDirs(".");
		char *dir;
		uint index;

		RumDirs = newList();

		foreach (dirs, dir, index)
			if (!_stricmp(RUM_DIR_EXT, getExt(dir)))
				addElement(RumDirs, (uint)strx(dir));

		releaseDim(dirs, 1);
	}

	sortJLinesICase(RumDirs);

	FoundRumDirs = newList();

	{
		char *dir;
		uint index;

		foreach (RumDirs, dir, index)
			SearchRumDir(dir);
	}

	cout("%9I64u %9I64u\n", TotalCount, TotalFileCount);

	writeLines(FOUNDLISTFILE, FoundRumDirs);
}
