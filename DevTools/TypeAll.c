/*
	TypeAll.exe [/P] (/LSS | [/S] TARGET-DIR)
*/

#include "C:\Factory\Common\all.h"

static int ShowPathMode;
static int IntoSubDirMode;

static void DoType(char *file)
{
	FILE *fp = fileOpen(file, "rt");
	char *line;

	if (ShowPathMode)
	{
		cout("===============================================================================\n");
		cout("* %s\n", file);
		cout("===============================================================================\n");
	}
	while (line = readLine(fp))
	{
		cout("%s\n", line);
		memFree(line);
	}
	fileClose(fp);
}
static void TypeAll(autoList_t *files)
{
	char *file;
	uint index;

	foreach (files, file, index)
	{
		DoType(file);
	}
}
static void TypeAll_Dir(char *dir)
{
	autoList_t *files = ( IntoSubDirMode ? lssFiles : lsFiles )(dir);

	sortJLinesICase(files);
	TypeAll(files);
	releaseDim(files, 1);
}
static void TypeAll_ListFile(char *listFile)
{
	autoList_t *files = readLines(listFile);

	TypeAll(files);
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/P"))
	{
		ShowPathMode = 1;
		goto readArgs;
	}
	if (argIs("/S"))
	{
		IntoSubDirMode = 1;
		goto readArgs;
	}

	if (argIs("/LSS"))
	{
		TypeAll_ListFile(FOUNDLISTFILE);
		return;
	}
	TypeAll_Dir(nextArg());
}
