/*
	FileList2MD5List.exe ファイルリストファイル 出力ファイル
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\md5Cache.h"

static void FileList2MD5List(char *fileListFile, char *outFile)
{
	FILE *rfp = fileOpen(fileListFile, "rt");
	FILE *wfp = fileOpen(outFile, "wt");
	char *file;

	while (file = readLine(rfp))
	{
		writeLine_x(wfp, md5Cache_makeHexHashFile(file));
		memFree(file);
	}
	fileClose(rfp);
	fileClose(wfp);
}
int main(int argc, char **argv)
{
	FileList2MD5List(getArg(0), getArg(1));
}
