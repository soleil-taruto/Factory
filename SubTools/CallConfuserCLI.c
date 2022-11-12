/*
	CallConfuserCLI.exe 入力ファイル 出力ファイル

		入力ファイル ... C#のコードをビルドしたexeファイルであること。
*/

#include "C:\Factory\Common\all.h"

#define CONFUSER_CLI_EXE "C:\\app\\ConfuserEx_bin\\Confuser.CLI.exe"
#define PROJ_TEMPLATE_FILE "C:\\Factory\\SubTools\\CallConfuserCLI_Proj.txt"

static void CopyDLL(char *rExeFile, char *wDir)
{
	char *rDir = getParent(rExeFile);
	autoList_t *files;
	char *file;
	uint index;

	files = lsFiles(rDir);

	foreach (files, file, index)
	{
		if (!_stricmp("DLL", getExt(file)))
		{
			char *wFile = changeRoot(strx(file), rDir, wDir);

			cout("CD.< %s\n", file);
			cout("CD.> %s\n", wFile);

			copyFile(file, wFile);

			memFree(wFile);
		}
	}
	releaseDim(files, 1);
	memFree(rDir);
}
static void CallConfuserCLI(char *rFile, char *wFile)
{
	char *projName;
	char *midDir;
	char *midFile;
	char *projFile;
	uint64 size1;
	uint64 size2;

	rFile = makeFullPath(rFile);
	wFile = makeFullPath(wFile);

	cout("< %s\n", rFile);
	cout("> %s\n", wFile);

	errorCase(!existFile(rFile));
	errorCase( existFile(wFile));
	errorCase(!creatable(wFile));

	projName = getLocal(rFile);
	projName = changeExt(projName, "");

	errorCase(!lineExp("<1,,__09AZaz>", projName));

	midDir = makeTempDir(NULL);
	midFile  = combine_cx(midDir, addExt(strx(projName), "exe"));
	projFile = combine_cx(midDir, addExt(strx(projName), "crproj"));

	copyFile(rFile, midFile);

	CopyDLL(rFile, midDir);

	{
		char *text = readText_b(PROJ_TEMPLATE_FILE);

		text = replaceLine(text, "$TARGET_EXE$", midFile, 0);

		writeOneLineNoRet_b_cx(projFile, text);
	}

	size1 = getFileSize(midFile);
	coExecute_x(xcout(CONFUSER_CLI_EXE " -n \"%s\"", projFile));
	size2 = getFileSize(midFile);

	cout("FILE SIZE: %I64u -> %I64u (%.3f)\n", size1, size2, (double)size2 / size1);

//	errorCase((double)size2 / size1 < 1.1); // 難読化するとファイルサイズは4倍くらいになるっぽい。1割も増えてないなら、何かおかしいと見る。<-- ルールによる

	copyFile(midFile, wFile);

	memFree(rFile);
	memFree(wFile);
	memFree(projName);
	recurRemoveDir_x(midDir);
	memFree(midFile);
	memFree(projFile);
}
int main(int argc, char **argv)
{
	errorCase(!existFile(CONFUSER_CLI_EXE));

	CallConfuserCLI(getArg(0), getArg(1));
}
