/*
	KCameraópÅAâÊëúçiÇËçûÇ›
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\bitList.h"

#define MARGIN 3

static void RemoveUnwantedFiles(autoList_t *files)
{
	char *file;
	uint index;

	foreach(files, file, index)
		if(!_stricmp("log", getExt(file)))
			*file = '\0';

	trimLines(files);
}
static int IsDx(char *str)
{
	char *p = mbs_stristr(str, "D");

	return p && p[1] != '0' && m_isdecimal(p[1]);
}
static void PickOutDx(char *dir)
{
	autoList_t *files = lsFiles(dir);
	char *file;
	uint index;
	char *destDir = makeFreeDir();
	char *destDir2;
	bitList_t *copyFlags;
	int copied = 0;

	coExecute_x(xcout("C:\\Factory\\SubTools\\OrderStamp.exe \"%s\"", dir)); // Ç¬Ç¢Ç≈Ç…ÅA

	addCwd(dir);
	eraseParents(files);
	RemoveUnwantedFiles(files);
	sortJLinesICase(files);

	destDir2 = combine_cx(destDir, xcout("%s_PODx", getLocal(dir)));
	createDir(destDir2);

	copyFlags = newBitList_A(getCount(files));

	foreach(files, file, index)
	{
		if(IsDx(file))
		{
			sint center = (sint)index;
			sint s;
			sint e;
			sint i;

			s = center - MARGIN;
			e = center + MARGIN;

			for(i = s; i <= e; i++)
			{
				if(0 <= i && i < (sint)getCount(files))
				{
					uint ii = (uint)i;

					putBit(copyFlags, ii, 1);
				}
			}
		}
	}
	foreach(files, file, index)
	{
		if(refBit(copyFlags, index))
		{
			char *rFile = combine(dir, file);
			char *wFile = combine(destDir2, file);

			cout("< %s\n", rFile);
			cout("> %s\n", wFile);

			copyFile(rFile, wFile);
			copied = 1;

			memFree(rFile);
			memFree(wFile);
		}
	}
	if(copied)
	{
		coExecute_x(xcout("C:\\Factory\\SubTools\\OrderStamp.exe \"%s\"", destDir2));
		coExecute_x(xcout("START \"\" \"%s\"", destDir));
	}
	releaseDim(files, 1);
	memFree(destDir);
	memFree(destDir2);
}
int main(int argc, char **argv)
{
	PickOutDx(nextArg());
}
