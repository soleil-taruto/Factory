#include "C:\Factory\Common\all.h"

#define R_ROOT_DIR "C:\\Factory"

static int IsTargetFile(char *file)
{
	char *ext = getExt(file);

	return
		!_stricmp(ext, "c") ||
		!_stricmp(ext, "h") ||
		!_stricmp(ext, "txt");
}
static void Main2(void)
{
	autoList_t *files = lssFiles(R_ROOT_DIR);
	char *file;
	uint index;
	char *wRootDir_P = makeFreeDir();
	char *wRootDir;

	wRootDir = combine(wRootDir_P, "Factory_mochi");
	createDir(wRootDir);

	foreach (files, file, index)
	{
		if (IsTargetFile(file))
		{
			char *wFile = changeRoot(strx(file), R_ROOT_DIR, wRootDir);

			createPath(wFile, 'X');
			copyFile(file, wFile);

			{
				uint64 crTime;
				uint64 wrTime;

				getFileStamp(file, &crTime, NULL, &wrTime);

				setFileStamp(wFile, crTime, 0, wrTime);
			}

			memFree(wFile);
		}
	}
	releaseDim(files, 1);

	coExecute_x(xcout("START %s", wRootDir_P));

	memFree(wRootDir_P);
	memFree(wRootDir);
}
int main(int argc, char **argv)
{
	Main2();
}
