/*
	※パスの大文字・小文字も区別する。
*/

#include "C:\Factory\Common\all.h"

static autoList_t *ReportLines;
static autoList_t *ReportLines_Ent;

static void Report(char *path, char *comment)
{
	addElement(ReportLines, (uint)xcout("%s*%s", comment, path));
}
static void Report_Ent_x(char *line)
{
	addElement(ReportLines_Ent, (uint)line);
}
static char *B_ReadLine(FILE *fp)
{
	autoBlock_t *buff = newBlock();

	for (; ; )
	{
		int chr = readChar(fp);

		if (chr == EOF)
			break;

		if (0x20 <= chr && chr <= 0x7e && chr != '\\')
			ab_addChar(buff, chr);
		else
			ab_addLine_x(buff, xcout("\\x%02x", chr));

		if (chr == '\n')
			break;
	}
	if (!getSize(buff))
	{
		releaseAutoBlock(buff);
		return NULL;
	}
	return unbindBlock2Line(buff);
}
static autoList_t *B_ReadLines(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	autoList_t *lines = newList();
	char *line;

	while (line = B_ReadLine(fp))
	{
		addElement(lines, (uint)line);
	}
	fileClose(fp);
	return lines;
}
static void CheckDiffFile(char *file1, char *file2, char *cmnRelPath)
{
	autoList_t *lines1 = B_ReadLines(file1);
	autoList_t *lines2 = B_ReadLines(file2);
	uint index;

	Report_Ent_x(xcout("%s", cmnRelPath));

	for (index = 0; index < getCount(lines1) || index < getCount(lines2); index++)
	{
		char *line1 = (char *)refElement(lines1, index);
		char *line2 = (char *)refElement(lines2, index);

		if (!line1 || !line2 || strcmp(line1, line2))
		{
			if (line1) Report_Ent_x(xcout("\t< %05u [%s]", index + 1, line1));
			if (line2) Report_Ent_x(xcout("\t> %05u [%s]", index + 1, line2));
		}
	}
	releaseDim(lines1, 1);
	releaseDim(lines2, 1);
}
static void DoReport(void)
{
	char *repFile1 = getOutFile("Report.txt");
	char *repFile2 = getOutFile("Report_Ent.txt");

	shootingStarLines(ReportLines);

	writeLines(repFile1, ReportLines);
	writeLines(repFile2, ReportLines_Ent);

	memFree(repFile1);
	memFree(repFile2);

	openOutDir();
}
static void Main2(char *rootDir1, char *rootDir2)
{
	rootDir1 = makeFullPath(rootDir1);
	rootDir2 = makeFullPath(rootDir2);
LOGPOS();

	{
		autoList_t *dirs1 = lssDirs(rootDir1);
		autoList_t *dirs2 = lssDirs(rootDir2);
		autoList_t *mDirs;
		char *dir;
		uint index;

		changeRoots(dirs1, rootDir1, NULL);
		changeRoots(dirs2, rootDir2, NULL);

		mDirs = mergeLines(dirs1, dirs2);

		foreach (dirs1, dir, index)
			Report(dir, "Left only directory");

		foreach (dirs2, dir, index)
			Report(dir, "Right only directory");

		foreach (mDirs, dir, index)
			Report(dir, "");

LOGPOS();
		releaseDim(dirs1, 1);
		releaseDim(dirs2, 1);
		releaseDim(mDirs, 1);
	}
LOGPOS();

	{
		autoList_t *files1 = lssFiles(rootDir1);
		autoList_t *files2 = lssFiles(rootDir2);
		autoList_t *mFiles;
		char *file;
		uint index;

		changeRoots(files1, rootDir1, NULL);
		changeRoots(files2, rootDir2, NULL);

		mFiles = mergeLines(files1, files2);

		foreach (files1, file, index)
			Report(file, "Left only file");

		foreach (files2, file, index)
			Report(file, "Right only file");

		foreach (mFiles, file, index)
		{
			char *file1 = combine(rootDir1, file);
			char *file2 = combine(rootDir2, file);

			if (isSameFile(file1, file2))
			{
				Report(file, "");
			}
			else
			{
				Report(file, "Different file");

				CheckDiffFile(file1, file2, file);
			}
			memFree(file1);
			memFree(file2);
		}
LOGPOS();
		releaseDim(files1, 1);
		releaseDim(files2, 1);
		releaseDim(mFiles, 1);
	}

LOGPOS();
	DoReport();
LOGPOS();

	memFree(rootDir1);
LOGPOS();
	memFree(rootDir2);
LOGPOS();
}
int main(int argc, char **argv)
{
	ReportLines = newList();
	ReportLines_Ent = newList();

	if (hasArgs(2))
	{
		Main2(getArg(0), getArg(1));
	}
	else
	{
		Main2("C:\\Dev", "C:\\Dev2");
	}
}
