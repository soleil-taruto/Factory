/*
	UpdatingCopy.exe 入力側ディレクトリ 出力側ディレクトリ

		出力側ディレクトリ

			"*(P)" をルートディレクトリから見て第２階層のディレクトリのローカル名に置き換える。例："C:\\aaaa\\bbbb\\cccc" -> bbbb
			"*(C)" をカレントディレクトリのローカル名に置き換える。
*/

#include "C:\Factory\Common\all.h"

static char *x_DirFilter(char *dir)
{
	autoList_t *pTkns = tokenizeYen_x(getCwd());

	errorCase(getCount(pTkns) < 4); // "C:\\Dev\\PPPP\\x99999999_CCCC" と同じか、これよりも深くなければならない。

	dir = strx(dir);

	dir = replaceLine(dir, "*(P)", getLine(pTkns, 2), 0); // 第2階層のローカル名
	dir = replaceLine(dir, "*(C)", getLine(pTkns, getCount(pTkns) - 1), 0); // カレントDIRのローカル名

	releaseDim(pTkns, 1);

	return dir;
}
static void CheckWDir(char *wDir)
{
	if (!existDir(wDir))
	{
		cout("出力側ディレクトリを作成します。\n");
		cout("wDir: %s\n", wDir);

		createDir(wDir);
	}
}
static void RemoveOldZip(char *wDir)
{
	autoList_t *files = lsFiles(wDir);
	char *file;
	uint index;

	foreach (files, file, index)
		if (!_stricmp(getExt(file), "zip"))
			removeFile(file);

	releaseDim(files, 1);
}
static char *GetFirstZipFile(char *rDir)
{
	autoList_t *files = lsFiles(rDir);
	char *file;
	uint index;

	foreach (files, file, index)
		if (!_stricmp(getExt(file), "zip"))
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

	cout("< %s\n", rDir);
	cout("> %s\n", wDir);

	CheckWDir(wDir);

	errorCase(!existDir(rDir));
	errorCase(!existDir(wDir)); // 2bs

	LOGPOS();
	RemoveOldZip(wDir);
	LOGPOS();
	CopyZip(rDir, wDir);
	LOGPOS();

	memFree(wDir);
}
