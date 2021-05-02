/*
	UpdatingCopy.exe 入力側ディレクトリ 出力側ディレクトリ

		出力側ディレクトリ ... "*" をカレントディレクトリのローカル名に置き換える。
*/

#include "C:\Factory\Common\all.h"

#define SIMPLE_INDEX_HTML_FIRST_LINE "<!-- generated by https://github.com/soleil-taruto/Factory/blob/main/SubTools/SimpleIndex.c -->"

static char *x_DirFilter(char *dir)
{
	return replaceLine(strx(dir), "*", getLocal(getCwd()), 0); // g
}
static void CheckWDir(char *wDir)
{
	if(!existDir(wDir))
	{
		cout("出力側ディレクトリを作成します。\n");
		cout("wDir: %s\n", wDir);

		createDir(wDir);
	}
}
static void NROZIC_Notice(char *indexHtmlFile)
{
	writeOneLineNoRet_b(indexHtmlFile, "<h1>UNDER MAINTENANCE</h1>");

	addCwd("C:\\be\\Web");
	{
		coExecute("Push.bat");
	}
	unaddCwd();

//	coSleep(20000); // 20秒
//	coSleep(30000); // 30秒
	coSleep(60000); // 1分
}
static void NROZIC_NoticeIfCan(char *indexHtmlFile)
{
	char *firstLine = readFirstLine(indexHtmlFile);

	LOGPOS();

	if(!strcmp(firstLine, SIMPLE_INDEX_HTML_FIRST_LINE))
	{
		LOGPOS();
		NROZIC_Notice(indexHtmlFile);
		LOGPOS();
	}
	LOGPOS();
	memFree(firstLine);
}
static void NoticeRemoveOldZipIfCan(char *wDir)
{
	autoList_t *files = lsFiles(wDir);
	char *file;
	uint index;
	char *indexHtmlFile;

	LOGPOS();

	foreach(files, file, index)
	{
		if(!_stricmp(getLocal(file), "index.html"))
		{
			LOGPOS();
			NROZIC_NoticeIfCan(file);
			LOGPOS();
			break;
		}
	}
	LOGPOS();
	releaseDim(files, 1);
}
static void RemoveOldZip(char *wDir)
{
	autoList_t *files = lsFiles(wDir);
	char *file;
	uint index;

	foreach(files, file, index)
	{
		if(!_stricmp(getExt(file), "zip"))
		{
#if 1
			removeFile(file);
#else // old
			if(getFileAttr_Hidden(file)) // ? 削除マーク済み
			{
				removeFile(file);
			}
			else
			{
				setFileAttr(file, 1, 1, 0, 0); // 削除マーク
			}
#endif
		}
	}
	releaseDim(files, 1);
}
static char *GetFirstZipFile(char *rDir)
{
	autoList_t *files = lsFiles(rDir);
	char *file;
	uint index;

	foreach(files, file, index)
		if(!_stricmp(getExt(file), "zip"))
			break;

	errorCase_m(!file, ".zip ファイルが見つかりません。");

	file = strx(file);
	releaseDim(files, 1);
	return file;
}
static void CopyZip(char *rDir, char *wDir)
{
	char *rFile = GetFirstZipFile(rDir);
	char *wFile;

	wFile = combine(wDir, getLocal(rFile));

	removeFileIfExist(wFile); // 隠し属性対策 -- 隠し属性だと上書き出来ない。でも削除は出来る。
	copyFile(rFile, wFile);

	memFree(rFile);
	memFree(wFile);
}
int main(int argc, char **argv)
{
	char *rDir;
	char *wDir;
	char *oldPrefix;

	rDir = nextArg();
	wDir = nextArg();

	wDir = x_DirFilter(wDir);

	CheckWDir(wDir);

	errorCase(!existDir(rDir));
	errorCase(!existDir(wDir)); // 2bs

	LOGPOS();
	NoticeRemoveOldZipIfCan(wDir);
	LOGPOS();
	RemoveOldZip(wDir);
	LOGPOS();
	CopyZip(rDir, wDir);
	LOGPOS();

	memFree(wDir);
}
