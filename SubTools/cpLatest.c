/*
	cpLatest.exe コピー元DIRを検索するDIR コピー元ローカルファイル名 コピー先ファイル
*/

#include "C:\Factory\Common\all.h"

static void CopyLatest(char *rDir, char *rLocalFile, char *wFile)
{
	autoList_t *rSubDirs;
	char *rSubDir;
	char *rFile;

	errorCase(!existDir(rDir));
	errorCase(!existFile(wFile) && !creatable(wFile));

	rSubDirs = lsDirs(rDir);
	sortJLinesICase(rSubDirs);
	rSubDir = (char *)getLastElement(rSubDirs);
	rFile = combine(rSubDir, rLocalFile);

	cout("< %s\n", rFile);
	cout("> %s\n", wFile);

	copyFile(rFile, wFile);

	cout("done\n");

	releaseDim(rSubDirs, 1);
	memFree(rFile);
}
int main(int argc, char **argv)
{
	char *rDir;
	char *rLocalFile;
	char *wFile;

	rDir       = nextArg();
	rLocalFile = nextArg();
	wFile      = nextArg();

	CopyLatest(rDir, rLocalFile, wFile);
}
