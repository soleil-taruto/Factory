/*
	FolderIcon.exe [/C]

		/C ... 全てのアイコン設定を解除する。
*/

#include "C:\Factory\Common\all.h"

#define MAPPING_FILE "FolderIcon.conf"

static autoList_t *FIP_Dirs;
static autoList_t *FIP_IconFiles;

static void LoadFolderIconPairs(void)
{
	autoList_t *linesList = readResourceLinesList(MAPPING_FILE);
	autoList_t *lines;
	char *line;
	uint index;
	uint lines_index;

	FIP_Dirs = newList();
	FIP_IconFiles = newList();

	foreach (linesList, lines, lines_index)
	{
		char *iconFile = getLine(lines, 0);

		errorCase(m_isEmpty(iconFile));
		errorCase(_stricmp(getExt(iconFile), "ico"));
		errorCase(!existFile(iconFile));

		foreach (lines, line, index)
		if (index)
		{
			char *dir = line;

			cout("dir: %s\n", dir);

			errorCase(m_isEmpty(dir));
			errorCase(!isFairLocalPath(dir, 0));
			errorCase(!existDir(dir));

			addElement(FIP_Dirs, (uint)strx(dir));
			addElement(FIP_IconFiles, (uint)strx(iconFile));
		}
	}
	releaseDim(linesList, 2);
}
static void SetFolderIcon(char *dir, char *iconFile)
{
	char *iniFile = combine(dir, "desktop.ini");

	if (iconFile)
	{
		coExecute_x(xcout("ATTRIB -S -H \"%s\"", iniFile));

		{
			FILE *fp = fileOpen(iniFile, "wt");

			writeLine(fp, "[.ShellClassInfo]");
			writeLine_x(fp, xcout("IconResource=%s,0", iconFile));

			fileClose(fp);
		}

		coExecute_x(xcout("ATTRIB +S +H \"%s\"", iniFile));
		coExecute_x(xcout("ATTRIB +R \"%s\"", dir));
	}
	else
	{
		removeFileIfExist(iniFile);

		coExecute_x(xcout("ATTRIB -R \"%s\"", dir));
	}
	memFree(iniFile);
}

static int ClearMode;

static void ChangeFolderIcons(void)
{
	autoList_t *dirs = lsDirs(".");
	char *dir;
	uint index;

	eraseParents(dirs);

	foreach (dirs, dir, index)
	{
		uint fipIndex = findLineCase(FIP_Dirs, dir, 1);

		if (fipIndex < getCount(FIP_Dirs) && !ClearMode)
		{
			SetFolderIcon(dir, getLine(FIP_IconFiles, fipIndex));
		}
		else
		{
			SetFolderIcon(dir, NULL);
		}
	}
	releaseDim(dirs, 1);
}
static void Main2(void)
{
	LoadFolderIconPairs();
	ChangeFolderIcons();
}
int main(int argc, char **argv)
{
	if (argIs("/C"))
	{
		ClearMode = 1;
	}
	Main2();
}
