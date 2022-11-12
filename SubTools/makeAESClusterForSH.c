#include "C:\Factory\Common\all.h"

/*
	ローカル名が '_' で始まるファイル・ディレクトリはその配下も含めて無視する。
*/
static int IsIgnoreRelFile(char *relFile)
{
	autoList_t *pTkns = tokenizeYen(relFile);
	char *pTkn;
	uint index;
	int ret = 0;

	foreach (pTkns, pTkn, index)
	{
		if (pTkn[0] == '_')
		{
			ret = 1;
			break;
		}
	}
	releaseDim(pTkns, 1);
	return ret;
}
static autoList_t *GetContents(char *rootDir)
{
	autoList_t *files = lssFiles(rootDir);
	char *file;
	uint index;

	changeRoots(files, rootDir, NULL);

	foreach (files, file, index)
		if (IsIgnoreRelFile(file))
			file[0] = '\0';

	trimLines(files);
	sortJLinesICase(files);
	return files;
}
static autoList_t *GetFileList(char *rootDir, autoList_t *contents, char *contentsFile)
{
	autoList_t *files = newList();
	char *content;
	uint index;

	addElement(files, (uint)strx(contentsFile));

	foreach (contents, content, index)
		addElement(files, (uint)combine(rootDir, content));

	return files;
}
int main(int argc, char **argv)
{
	char *rootDir;
	char *clusterFile;
	char *rawKey;
	char *firstTweek;
	char *contentsFile;
	char *fileListFile;
	autoList_t *contents;
	autoList_t *fileList;

	rootDir = nextArg();
	clusterFile = nextArg();
	rawKey = nextArg();
	firstTweek = nextArg();

	rootDir = makeFullPath(rootDir);
	contentsFile = makeTempPath(NULL);
	fileListFile = makeTempPath(NULL);
	contents = GetContents(rootDir);
	fileList = GetFileList(rootDir, contents, contentsFile);

	writeLines(contentsFile, contents);
	writeLines(fileListFile, fileList);

	coExecute(xcout("C:\\Factory\\SubTools\\makeAESCluster.exe \"%s\" \"%s\" \"%s\" \"%s\"", fileListFile, clusterFile, rawKey, firstTweek));

	removeFile(contentsFile);
	removeFile(fileListFile);

	memFree(rootDir);
	memFree(contentsFile);
	memFree(fileListFile);
	releaseDim(contents, 1);
	releaseDim(fileList, 1);
}
