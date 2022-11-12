/*
	DLIndex.exe [/S] ルートDIR

	----
	テストデータ

	C:\wb2\20200604_DLIndexTest

	----
	フォルダ構成

		<実行時のカレントDIR>   **1
		|
		+--define.txt 1
		|
		+--index.html.txt 1
		|
		+--app_index.html.txt 1
		|
		+--app_allver.html.txt 1
		|
		+--extra_sourcecodelink.html.txt 1

		<ルートDIR> 1
		|
		+--<APP> 1*
		|  |
		|  +--<REV> 1*
		|  |  |
		|  |  +--<DLFILE> 1
		|  |  |
		|  |  +--<DLFILE>.md5 <- out
		|  |
		|  +--description.txt 0*1
		|  |
		|  +--sourcecodelink.txt 0*1
		|  |
		|  +--allver.html <- out
		|  |
		|  +--index.html <- out
		|
		+--index.html <- out
		|
		+--newest.html <- out

		APP
			アプリ名
			空白無しASCII文字列であること。
			'_' で始まるフォルダはスキップする。

		REV
			リビジョン
			空白無しASCII文字列であること。
			strcmp()による比較で新旧を判断する。

		DLFILE
			公開ファイル名
			空白無しASCII文字列であること。
			リビジョン毎に名前が異なっても良い。

		description.txt
			アプリの説明文
			CP932/MultiLine

		sourcecodelink.txt
			ソースコードへのリンク(URL)
			複数可・改行区切り
			CP932/MultiLine

		out ... このプログラムが出力するファイル

		**1 ... 実行時のカレントDIRから見つからない場合は、実行ファイルと同じ場所を見る。そこにも無ければエラー。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

static int IsAsciiStr(char *str)
{
	char *p;

	for (p = str; *p; p++)
	{
		if (!m_isRange(*p, '\x21', '\x7e'))
		{
			return 0;
		}
	}
	return 1;
}
static int IsHexStr(char *str)
{
	char *p;

	for (p = str; *p; p++)
	{
		if (
			!m_isRange(*p, '0', '9') &&
			!m_isRange(m_tolower(*p), 'a', 'f')
			)
		{
			return 0;
		}
	}
	return 1;
}

#define DEFINE_FILE "define.txt"
#define ROOTINDEX_FMTFILE "index.html.txt"
#define APPINDEX_FMTFILE "app_index.html.txt"
#define APPALLVER_FMTFILE "app_allver.html.txt"
#define EXTRA_SOURCECODELINK_FMTFILE "extra_sourcecodelink.html.txt"

#define DESCRIPTION_FILE "description.txt"
#define SOURCECODELINK_FILE "sourcecodelink.txt"

#define INDEX_FILE "index.html"
#define ALLVER_FILE "allver.html"
#define NEWEST_FILE "newest.txt"

static char *GetAppResourceFile(char *localFile)
{
	char *file;

	if (existFile(localFile))
		file = makeFullPath(localFile);
	else
		file = combine(getSelfDir(), localFile);

	cout("app_res_file: %s\n", file);

	return file;
}

// ---- define ----

static autoList_t *DefineKeys;
static autoList_t *DefineValues;

static void LoadDefineFile(void)
{
	autoList_t *lines = readResourceLines_x(GetAppResourceFile(DEFINE_FILE));
	uint index;

	errorCase(getCount(lines) & 1);

	DefineKeys = newList();
	DefineValues = newList();

	for (index = 0; index < getCount(lines); )
	{
		addElement(DefineKeys,   getElement(lines, index++));
		addElement(DefineValues, getElement(lines, index++));
	}
	releaseAutoList(lines);
}
static char *ReplaceAllDefine(char *str)
{
	char *def_key;
	uint index;

	foreach (DefineKeys, def_key, index)
	{
		char *def_value = getLine(DefineValues, index);

		str = replaceLine(str, def_key, def_value, 0);
	}
	return str;
}

// ----

typedef struct RevInfo_st
{
	char *Rev;
	char *DLFile;
	uint64 Size;
	char *Hash;
}
RevInfo_t;

typedef struct AppInfo_st
{
	char *AppName;
	autoList_t *RevInfos;
	char *Description;
	autoList_t *SourceCodeLinks;
}
AppInfo_t;

static autoList_t *AppInfos;
static int SkipHashCheckFlag;

static char *LoadFmtFile(char *localFile)
{
	char *fmtFile = GetAppResourceFile(localFile);
	char *fmt;

	fmt = untokenize_xc(readLines(fmtFile), "\n");
	memFree(fmtFile);
	return fmt;
}
static void LoadRevInfos(char *rootDir, autoList_t *riList)
{
	autoList_t *revDirs = lsDirs(rootDir);
	char *dir;
	uint index;

	foreach (revDirs, dir, index)
	{
		RevInfo_t *ri = nb_(RevInfo_t);

		ri->Rev = strx(getLocal(dir));
		errorCase(!IsAsciiStr(ri->Rev));

		{
			autoList_t *dlFiles = lsFiles(dir);
			char *dlFile;
			char *dlHashFile;

			if (getCount(dlFiles) == 1)
			{
				dlFile = getLine(dlFiles, 0);
				dlHashFile = addExt(strx(dlFile), "md5");
			}
			else if (getCount(dlFiles) == 2)
			{
				if (strlen(getLine(dlFiles, 1)) < strlen(getLine(dlFiles, 0)))
					swapElement(dlFiles, 0, 1);

				dlFile = getLine(dlFiles, 0);
				dlHashFile = getLine(dlFiles, 1);
			}
			else // ? ファイルが無い || 余計なファイルがある
			{
				error();
			}
			cout("dlFile: %s\n", dlFile);
			cout("dHFile: %s\n", dlHashFile);

			{
				char *wkPath = addExt(strx(dlFile), "md5");
				errorCase(_stricmp(wkPath, dlHashFile)); // ? dlFile + .md5 != dlHashFile
				memFree(wkPath);
			}

			ri->DLFile = strx(getLocal(dlFile));
			cout("file: %s\n", ri->DLFile);
			errorCase(!IsAsciiStr(ri->DLFile));

			ri->Size = getFileSize(dlFile);
			cout("size: %I64u\n", ri->Size);

			if (existFile(dlHashFile)) // チェック
			{
				ri->Hash = readFirstLine(dlHashFile);
				cout("rdHash: %s\n", ri->Hash);
				errorCase(!IsHexStr(ri->Hash));

				if (!SkipHashCheckFlag)
				{
					char *mkHash = md5_makeHexHashFile(dlFile);

					cout("mkHash: %s\n", mkHash);
					errorCase(_stricmp(ri->Hash, mkHash));
					memFree(mkHash);

					goto recreateMd5;
				}
			}
			else // 生成
			{
			recreateMd5:
				ri->Hash = md5_makeHexHashFile(dlFile);
				cout("hash: %s\n", ri->Hash);
				writeOneLine(dlHashFile, ri->Hash);
			}
			releaseDim(dlFiles, 1);
		}
		addElement(riList, (uint)ri);
	}
	releaseDim(revDirs, 1);
}
static void LoadAppInfos(char *rootDir)
{
	autoList_t *appDirs = lsDirs(rootDir);
	char *dir;
	uint index;

	// 除外すべきフォルダを除去
	{
		foreach (appDirs, dir, index)
		{
			if (getLocal(dir)[0] == '_') // ? APP が '_' で始まる。
			{
				dir[0] = '\0';
			}
		}
		trimLines(appDirs);
	}
	sortJLinesICase(appDirs); // ABC順

	AppInfos = newList();

	foreach (appDirs, dir, index)
	{
		AppInfo_t *ai = nb_(AppInfo_t);

		cout("%s\n", dir);

		ai->AppName = strx(getLocal(dir));
		errorCase(!IsAsciiStr(ai->AppName));
		ai->RevInfos = newList();

		LoadRevInfos(dir, ai->RevInfos);
#if 1
		if (!getCount(ai->RevInfos))
		{
			RevInfo_t *ri = nb_(RevInfo_t);

			ri->Rev = strx("0000.000.00000");
			ri->DLFile = strx("none");
			ri->Size = 0;
			ri->Hash = strx("00000000000000000000000000000000");

			addElement(ai->RevInfos, (uint)ri);
		}
#else
		errorCase_m(!getCount(ai->RevInfos), "リリースされたリビジョンが１つもありません。");
#endif

		{
			char *file = combine(dir, DESCRIPTION_FILE);

			if (existFile(file))
				ai->Description = untokenize_xc(readLines(file), "\n");
			else
				ai->Description = strx("説明文がありません。");
//				ai->Description = strx("説明文がありません。\n改行\n改行\n改行");

			memFree(file);
			cout("description: %s\n", ai->Description);
		}

		{
			char *file = combine(dir, SOURCECODELINK_FILE);

			if (existFile(file))
				ai->SourceCodeLinks = readResourceLines(file);
			else
				ai->SourceCodeLinks = newList();

			memFree(file);
			cout("sourcecodelink_num: %u\n", getCount(ai->SourceCodeLinks));
		}

		addElement(AppInfos, (uint)ai);
	}
	releaseDim(appDirs, 1);
	errorCase(!getCount(AppInfos));
}
static void MakeRootIndex(char *rootDir, char *rootIndexFmt)
{
	char *indexFile = combine(rootDir, INDEX_FILE);
	char *strAppList;

	{
		autoList_t *lines = newList();
		AppInfo_t *ai;
		uint index;

		foreach (AppInfos, ai, index)
		{
			char *description = strx(ai->Description);

			description = replaceLine(description, "\n", "<br/>\n", 0);

			addElement(lines, (uint)strx("<tr>"));
			addElement(lines, (uint)xcout("<td class=\"appname\"><a href=\"%s/%s\">%s</a></td>", ai->AppName, INDEX_FILE, ai->AppName));
			addElement(lines, (uint)xcout("<td class=\"description\">%s</td>", description));
			addElement(lines, (uint)strx("</tr>"));

			memFree(description);
		}
		strAppList = untokenize_xc(lines, "\n");
	}

	rootIndexFmt = strx(rootIndexFmt);
	rootIndexFmt = replaceLine(rootIndexFmt, "*app-list*", strAppList, 0);
	rootIndexFmt = ReplaceAllDefine(rootIndexFmt);

	writeOneLine(indexFile, rootIndexFmt);

	memFree(indexFile);
	memFree(strAppList);
	memFree(rootIndexFmt);
}
static sint CompRevInfo(RevInfo_t *ri1, RevInfo_t *ri2)
{
	return strcmp(ri1->Rev, ri2->Rev);
}
static void MakeAppIndex(char *rootDir, AppInfo_t *ai, char *appIndexFmt, char *appAllVerFmt)
{
	autoList_t *dlLinkList = newList();
	RevInfo_t *ri;
	uint index;
	char *strAppList;

	rapidSort(ai->RevInfos, (sint (*)(uint, uint))CompRevInfo);
	reverseElements(ai->RevInfos); // 新 -> 旧

	foreach (ai->RevInfos, ri, index)
	{
		addElement(dlLinkList, (uint)xcout("%s/%s", ri->Rev, ri->DLFile));
	}
//	rapidSortLines(dlLinkList);
//	reverseElements(dlLinkList); // 新 -> 旧

	appIndexFmt = strx(appIndexFmt);
	appIndexFmt = replaceLine(appIndexFmt, "*app-name*", ai->AppName, 0);
	appIndexFmt = replaceLine(appIndexFmt, "*newest*", getLine(dlLinkList, 0), 0);
	appIndexFmt = replaceLine(appIndexFmt, "*newest-rev*", ((RevInfo_t *)getElement(ai->RevInfos, 0))->Rev, 0);
	appIndexFmt = replaceLine(appIndexFmt, "*newest-md5*", ((RevInfo_t *)getElement(ai->RevInfos, 0))->Hash, 0);

	{
		char *description = strx(ai->Description);
		description = replaceLine(description, "\n", "<br/>\n", 0);
		appIndexFmt = replaceLine(appIndexFmt, "*description*", description, 0);
		memFree(description);
	}

	{
		char *sourcecodelink;

		if (getCount(ai->SourceCodeLinks))
			sourcecodelink = getLine(ai->SourceCodeLinks, 0);
		else
			sourcecodelink = "javascript:alert('リンク情報がありません。')";

		appIndexFmt = replaceLine(appIndexFmt, "*sourcecodelink*", sourcecodelink, 0);
	}

	{
		char *fmt = LoadFmtFile(EXTRA_SOURCECODELINK_FMTFILE);
		char *buff = strx("");

		for (index = 1; index < getCount(ai->SourceCodeLinks); index++)
		{
			char *tmp = strx(fmt);
			char *title = xcout("#%u", index + 1);

			tmp = replaceLine(tmp, "*sourcecodelink*", getLine(ai->SourceCodeLinks, index), 0);
			tmp = replaceLine(tmp, "*title*", title, 0);

			buff = addLine(buff, tmp);

			memFree(tmp);
			memFree(title);
		}
		memFree(fmt);

		appIndexFmt = replaceLine(appIndexFmt, "*extra-sourcecodelink*", buff, 0);

		memFree(buff);
	}

	appIndexFmt = ReplaceAllDefine(appIndexFmt);

	{
		char *indexFile = combine_xc(combine(rootDir, ai->AppName), INDEX_FILE);
		writeOneLine(indexFile, appIndexFmt);
		memFree(indexFile);
	}

	memFree(appIndexFmt);

	{
		autoList_t *lines = newList();
		AppInfo_t *dlLink;
		uint index;

		foreach (dlLinkList, dlLink, index)
		{
			RevInfo_t *ri = (RevInfo_t *)getElement(ai->RevInfos, index);

			addElement(lines, (uint)xcout(
				"<a href=\"%s\">%s %s (md5:%s, size:%s bytes)</a><br/>"
				,dlLink
				,ai->AppName
				,ri->Rev
				,ri->Hash
				,c_thousandComma(xcout("%I64u", ri->Size))
				));
		}
		strAppList = untokenize_xc(lines, "\n");
	}

	appAllVerFmt = strx(appAllVerFmt);
	appAllVerFmt = replaceLine(appAllVerFmt, "*app-name*", ai->AppName, 0);
	appAllVerFmt = replaceLine(appAllVerFmt, "*app-list*", strAppList, 0);
	appAllVerFmt = ReplaceAllDefine(appAllVerFmt);

	{
		char *allVerFile = combine_xc(combine(rootDir, ai->AppName), ALLVER_FILE);
		writeOneLine(allVerFile, appAllVerFmt);
		memFree(allVerFile);
	}

	memFree(appAllVerFmt);

	releaseDim(dlLinkList, 1);
	memFree(strAppList);
}
static void MakeNewestIndex(char *rootDir)
{
	char *newestFile = combine(rootDir, NEWEST_FILE);
	autoList_t *lines = newList();
	AppInfo_t *ai;
	uint index;

	foreach (AppInfos, ai, index)
	{
		RevInfo_t *ri = (RevInfo_t *)getElement(ai->RevInfos, 0);

		addElement(lines, (uint)xcout(
			"%s*%s*%I64u*%s"
			,ai->AppName
			,ri->Rev
			,ri->Size
			,ri->Hash
			));
	}
	shootingStarLines(lines);

	writeLines(newestFile, lines);

	memFree(newestFile);
	releaseDim(lines, 1);
}
void MakeDLIndex(char *rootDir)
{
	char *rootIndexFmt = LoadFmtFile(ROOTINDEX_FMTFILE);
	char *appIndexFmt = LoadFmtFile(APPINDEX_FMTFILE);
	char *appAllVerFmt = LoadFmtFile(APPALLVER_FMTFILE);

	LoadDefineFile();

	LoadAppInfos(rootDir);
	MakeRootIndex(rootDir, rootIndexFmt);

	{
		AppInfo_t *ai;
		uint index;

		foreach (AppInfos, ai, index)
		{
			MakeAppIndex(rootDir, ai, appIndexFmt, appAllVerFmt);
		}
	}

	MakeNewestIndex(rootDir);

	memFree(rootIndexFmt);
	memFree(appIndexFmt);
	memFree(appAllVerFmt);
}
int main(int argc, char **argv)
{
	char *rootDir;

readArgs:
	if (argIs("/S"))
	{
		SkipHashCheckFlag = 1;
		goto readArgs;
	}

	rootDir = makeFullPath(nextArg());
	cout("rootDir: %s\n", rootDir);
	errorCase(!existDir(rootDir));

	MakeDLIndex(rootDir);

	memFree(rootDir);
}
