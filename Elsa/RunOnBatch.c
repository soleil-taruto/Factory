/*
	RunOnBatch.exe 作業ディレクトリ 実行可能ファイル [コマンド引数]...
*/

#include "C:\Factory\Common\all.h"

static void RunOnBatch(char *workDir, char *runnableFile, char *arguments)
{
	char *batFile = makeTempPath("bat");

	writeOneLine_cx(batFile, xcout("%s %s", runnableFile, arguments));

	addCwd(workDir);
	coExecute(batFile);
	unaddCwd();

	removeFile(batFile);
	memFree(batFile);
}
int main(int argc, char **argv)
{
	char *workDir;
	char *runnableFile;
	char *arguments;

	workDir = nextArg();
	runnableFile = nextArg();
	arguments = untokenize_xc(allArgs(), " ");

	errorCase(!existDir(workDir));
	addCwd(workDir);
	errorCase(!existFile(runnableFile));
	unaddCwd();
	// arguments

	LOGPOS();
	RunOnBatch(workDir, runnableFile, arguments);
	LOGPOS();
}
