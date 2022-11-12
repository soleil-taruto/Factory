/*
	httpMonParse.exe INPUT-DIR OUTPUT-DIR

		INPUT-DIR  ... 入力ディレクトリ
		OUTPUT-DIR ... 出力ディレクトリ

	入力・出力ディレクトリは、存在するディレクトリであること。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Labo\Socket\libs\http\ExtToContentType.h"
#include "libs\HTTPParse.h"

static char *GetContentType(void)
{
	char *key;
	uint index;

	foreach (HttpDat.H_Keys, key, index)
	{
		if (!_stricmp(key, "Content-Type"))
		{
			return getLine(HttpDat.H_Values, index);
		}
	}
	return "XXX";
}
static char *GetContentExt(void)
{
	char *ret1;
	char *ret2;

	ret2 = httpContentTypeToExt(ret1 = GetContentType());

	cout("ret1: %s\n", ret1);
	cout("ret2: %s\n", ret2);

	return ret2;
}
static void FileParse(char *rFile, char *wDir)
{
	autoBlock_t *fileData = readBinary(rFile);
	char *wFileBase;
	char *wFile;
	FILE *wFp;
	char *key;
	uint index;

	errorCase(!HTTPParse(fileData));
	errorCase(HttpDat.EndPos != getSize(fileData));

	releaseAutoBlock(fileData);

	wFileBase = combine(wDir, getLocal(rFile));
	wFileBase = changeExt(wFileBase, "");

	// ---- Header ----

	wFile = xcout("%s-0.txt", wFileBase);
	wFp = fileOpen(wFile, "wt");

	writeLine(wFp, HttpDat.H_Request);

	foreach (HttpDat.H_Keys, key, index)
	{
		char *value = getLine(HttpDat.H_Values, index);

		writeLine_x(wFp, xcout("%s: %s", key, value));
	}
	fileClose(wFp);
	memFree(wFile);

	// ---- Body ----

	wFile = xcout("%s-1.%s", wFileBase, GetContentExt());
	writeBinary(wFile, HttpDat.Body);
	memFree(wFile);

	// ----

	memFree(wFileBase);
}
static void HTTPMonParse(char *rDir, char *wDir)
{
	autoList_t *files = lsFiles(rDir);
	char *file;
	uint index;

//	createDirIfNotExist(wDir);

	foreach (files, file, index)
	{
		FileParse(file, wDir);
	}
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	HTTPMonParse(getArg(0), getArg(1));
}
