#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoList_t *files = lssFiles("C:\\Dev");
	char *file;
	uint index;
	FILE *fp = fileOpen("C:\\temp\\AutoReleaseAll_0001.bat", "wt");

	writeLine(fp, "CD /D C:\\temp"); // 安全のため
	writeLine(fp, "SET zip_NoPause=1"); // C:\Factory\SubTools\zip.c 用
	writeLine(fp, "");

	sortJLinesICase(files);

	foreach (files, file, index)
	if (!_stricmp("AutoRelease.bat", getLocal(file)))
	{
		writeLine_x(fp, xcout("CD /D \"%s\"", c_getParent(file)));
		writeLine(fp, "C:\\Factory\\Tools\\wait.exe 3");
		writeLine(fp, "IF ERRORLEVEL 1 GOTO END");
		writeLine(fp, "CALL AutoRelease.bat /-P");
		writeLine(fp, "CD /D C:\\temp"); // 安全のため
		writeLine(fp, "");
	}
	writeLine(fp, ":END");
	writeLine(fp, "SET zip_NoPause="); // C:\Factory\SubTools\zip.c 用

	fileClose(fp);

	releaseDim(files, 1);
}
