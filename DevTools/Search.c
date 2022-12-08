/*
	検索対象 == カレントの配下

	Search.exe [/E SPEC-EXTS] [/I] [/T] [/SVN] [/8] [/-] FIND-PATTERN

		SPEC-EXTS ... 検索対象の拡張子を '.' 区切りで記述する。

			例) c.h.txt

			/E "" は拡張子無しに対応
			/E .txt や /E txt. は拡張子 txt と拡張子無しに対応
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"

static void FSeek(FILE *fp, uint64 index)
{
	fileSeek(fp, SEEK_SET, index);
}
static int IsTokenChar(int chr)
{
	return m_isdecimal(chr) || m_isalpha(chr) || chr == '_';
}
static void DispRange(FILE *fp, sint64 start, sint64 end, sint64 fileSize)
{
	sint64 index;
	int chr;
	int disped = 0;

	for (index = start; index <= end; index++)
	{
		if (0 <= index && index < fileSize)
		{
			if (!disped)
			{
				disped = 1;
				FSeek(fp, index);
			}
			chr = readChar(fp);

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

static autoList_t *SpecExts;
static int IgnoreCase;
static int TokenOnlyMode;

static char *FindPattern;
static autoList_t *FoundFiles;
static autoList_t *Sections;
static uint64 TotalCount;
static uint64 TotalFileCount;

static void SearchFile(char *file)
{
	uint64 fileSize;
	uint64 rIndex = 0;
	uint64 crcnt = 1;
	uint64 lfcnt = 1;
	uint matchidx = 0;
	FILE *fp;
	int chr;
	uint64 fndcnt = 0;
	char *ext;
	uint extndx;

	if (SpecExts)
	{
		foreach (SpecExts, ext, extndx)
			if (!_stricmp(ext, getExt(file)))
				break;

		if (!ext)
			return;
	}
	fileSize = getFileSize(file);
	fp = fileOpen(file, "rb");

	while (rIndex < fileSize)
	{
		chr = readChar(fp);
		rIndex++;

		if (IgnoreCase ? m_tolower(chr) == m_tolower(FindPattern[matchidx]) : chr == FindPattern[matchidx])
		{
			matchidx++;

			if (FindPattern[matchidx] == '\0')
			{
				if (TokenOnlyMode)
				{
					if (matchidx < rIndex) // ? マッチ部分の先頭は、ファイルの先頭ではない。
					{
						FSeek(fp, rIndex - matchidx - 1);
						chr = readChar(fp);
						FSeek(fp, rIndex);

						if (IsTokenChar(chr)) // ? マッチ部分の先頭は、トークンの先頭ではない。
						{
							goto resetmatch;
						}
					}
					if (rIndex < fileSize) // ? マッチ部分の終端は、ファイルの終端 (最後の１バイト) ではない。
					{
						chr = readChar(fp);
						FSeek(fp, rIndex);

						if (IsTokenChar(chr)) // ? マッチ部分の終端は、トークンの終端ではない。
						{
							goto resetmatch;
						}
					}
				}

				// Found

				if (!fndcnt)
				{
					cout("%s\n", file);
					addElement(FoundFiles, (uint)strx(file));
					TotalFileCount++;
				}
				fndcnt++;
				TotalCount++;
				cout("%9I64u %9I64u ", fndcnt, m_max(crcnt, lfcnt));

				{
					sint64 ndx1b;
					sint64 ndx1e;
					sint64 ndx2b;
					sint64 ndx2e;

					ndx2b = rIndex;
					ndx2e = ndx2b + 28;
					ndx1e = ndx2b - matchidx - 1;
					ndx1b = ndx1e - 28;

					DispRange(fp, ndx1b, ndx1e, (sint64)fileSize);
					cout(" ");
					DispRange(fp, ndx2b, ndx2e, (sint64)fileSize);
					cout("\n");

					/*
						ファイル名 + \t + 開始位置 + \t + 終了位置

							開始位置 ... このバイトを含む。
							終了位置 ... このバイトを含まない。
					*/
					addElement(Sections, (uint)xcout("%s\t%I64u\t%I64u", file, ndx1e + 1, ndx2b));

					FSeek(fp, rIndex);
				}

			resetmatch:
				matchidx = 0;
			}
		}
		else if (matchidx)
		{
			rIndex -= matchidx;
			matchidx = 0;

			FSeek(fp, rIndex);
		}
		else
		{
			     if (chr == '\r') crcnt++;
			else if (chr == '\n') lfcnt++;
		}
	}
	fileClose(fp);
}
int main(int argc, char **argv)
{
	autoList_t *paths;
	autoList_t files;
	char *file;
	uint index;
	int utf8mode = 0;

	antiSubversion = 1;

readArgs:
	if (argIs("/E")) // Extension
	{
		SpecExts = tokenize(nextArg(), '.');
		goto readArgs;
	}
	if (argIs("/I")) // Ignore case
	{
		IgnoreCase = 1;
		goto readArgs;
	}
	if (argIs("/T")) // Token only mode
	{
		TokenOnlyMode = 1;
		goto readArgs;
	}
	if (argIs("/SVN")) // .svn の配下も探す。
	{
		antiSubversion = 0;
		goto readArgs;
	}
	if (argIs("/8"))
	{
		utf8mode = 1;
		goto readArgs;
	}
	argIs("/-");

	FindPattern = nextArg();
	FoundFiles = newList();
	Sections = newList();

	errorCase(FindPattern[0] == '\0');

	if (utf8mode)
	{
		char *tmpFile = makeTempPath(NULL);

		writeOneLineNoRet_b(tmpFile, FindPattern);
		SJISToUTF8File(tmpFile, tmpFile);
		FindPattern = readText_b(tmpFile); // g
		removeFile(tmpFile);
		memFree(tmpFile);

		cout("UTF-8\n");
	}
	paths = lss(".");
	files = gndFollowElements(paths, lastDirCount);

	rapidSort(&files, (sint (*)(uint, uint))mbs_stricmp);

	foreach (&files, file, index)
	{
		SearchFile(file);
	}
	releaseDim(paths, 1);

	writeLines(FOUNDLISTFILE, FoundFiles);
	releaseDim(FoundFiles, 1);
	writeLines(SECTIONLISTFILE, Sections);
	releaseDim(Sections, 1);

//	cout("-------------------\n");
	cout("%9I64u %9I64u\n", TotalCount, TotalFileCount);
}
