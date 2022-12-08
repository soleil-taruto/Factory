/*
	md5TenLine.exe [TARGET-PATH]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

#define PART_LNUM 10

static char *GetPartHash(autoList_t *lines, uint bgnLIdx, uint endLIdx)
{
	char *file = makeTempPath(NULL);
	char *ret;

	{
		FILE *fp = fileOpen(file, "wt");
		uint index;

		for (index = bgnLIdx; index <= endLIdx; index++)
			writeLine(fp, getLine(lines, index));

		fileClose(fp);
	}

	ret = c_md5_makeHexHashFile(file);

	removeFile(file);
	memFree(file);
	return ret;
}
int main(int argc, char **argv)
{
	char *path = hasArgs(1) ? nextArg() : c_dropDirFile();
	autoList_t *files;
	char *file;
	uint index;

	if (existFile(path))
	{
		files = newList();
		addElement(files, (uint)strx(path));
	}
	else
	{
		files = lsFiles(path);
	}

	sortJLinesICase(files);

	cout("--------------------------------\n");

	foreach (files, file, index)
	{
		autoList_t *lines = readLines(file);
		uint bgnLIdx;

		cout("%s %s\n", c_md5_makeHexHashFile(file), getLocal(file));

		for (bgnLIdx = 0; bgnLIdx < getCount(lines); bgnLIdx += PART_LNUM)
		{
			uint endLIdx = bgnLIdx + PART_LNUM;

			m_minim(endLIdx, getCount(lines));
			endLIdx--;

			cout("%s <%04u-%04u>\n", GetPartHash(lines, bgnLIdx, endLIdx), bgnLIdx + 1, endLIdx + 1);
		}
		releaseDim(lines, 1);

		cout("--------------------------------\n");
	}
	releaseDim(files, 1);
}
