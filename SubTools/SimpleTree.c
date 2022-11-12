/*
	SimpleTree.exe ルートDIR 出力ファイル
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\URL.h"

static FILE *OutFp;

static int IsIgnorePath(char *path, char *localPath)
{
	// 表示しないパスの条件は SimpleIndex に合わせる。
	// 但し index.html は表示する。

	if (existDir(path)) // ディレクトリ
	{
		if (localPath[0] == '_')
			return 1;
	}
	else // ファイル
	{
		if (
			!_stricmp(localPath, "_index.html_") ||
			!_stricmp(localPath, "_header.html_") ||
			!_stricmp(localPath, "_footer.html_")
			)
			return 1;
	}
	return 0;
}
static int IsNoLinkPath(char *path)
{
	return existFile(path) && getFileSize(path) == 0;
}
static void Search(char *rootDir, uint depth, char *pathPrefix)
{
	autoList_t *paths = ls(rootDir);
	char *path;
	uint index;
	uint ndx;
	static int empLnSw = 0;

	sortJLinesICase(paths);

	foreach (paths, path, index)
	{
		char *localPath = getLocal(path);

		if (IsIgnorePath(path, localPath))
			continue;

		if (empLnSw)
		{
			writeChar(OutFp, '\n'); // 空行
			empLnSw = 0;
		}
		for (ndx = 0; ndx < depth; ndx++)
			writeChar(OutFp, '\t');

		if (IsNoLinkPath(path))
		{
			writeLine(OutFp, localPath);
		}
		else
		{
			char *tmp1;
			char *tmp2;

			writeLine_x(OutFp, xcout("<a style=\"text-decoration: none\" href=\"%s/%s\">%s</a>", tmp1 = urlEncoder(pathPrefix), tmp2 = urlEncoder(localPath), localPath));

			memFree(tmp1);
			memFree(tmp2);
		}
		if (existDir(path))
		{
			char *nextPathPrefix = xcout("%s/%s", pathPrefix, localPath);

			Search(path, depth + 1, nextPathPrefix);

			memFree(nextPathPrefix);
		}
	}
	releaseDim(paths, 1);
	empLnSw = 1;
}
static void WriteHeader(void)
{
	writeLine(OutFp, "<html><head><meta charset=\"Shift_JIS\"></head><body><h1>TREE</h1><pre style=\"font-family: Meiryo\">");
}
static void WriteFooter(void)
{
	writeLine(OutFp, "</pre></body></html>");
}
static void SimpleTree(char *rootDir, char *outFile)
{
	LOGPOS();

	rootDir = makeFullPath(rootDir);
	outFile = makeFullPath(outFile);

	cout("< %s\n", rootDir);
	cout("> %s\n", outFile);

	errorCase(!existDir(rootDir));
	errorCase(!existFile(outFile) && !creatable(outFile));

	OutFp = fileOpen(outFile, "wt");

	WriteHeader();
	Search(rootDir, 0, ".");
	WriteFooter();

	fileClose(OutFp);
	OutFp = NULL;

	memFree(rootDir);
	memFree(outFile);

	LOGPOS();
}
int main(int argc, char **argv)
{
	SimpleTree(getArg(0), getArg(1));
}
