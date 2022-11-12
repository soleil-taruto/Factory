#include "C:\Factory\Common\all.h"

static void DoSlim(char *rootDir)
{
	autoList_t *paths = ls(rootDir);
	char *path;
	uint index;

	foreach (paths, path, index)
	{
		if (existDir(path))
		{
			if (!_stricmp(getExt(path), "rum"))
				coExecute_x(xcout("C:\\Factory\\Tools\\RDMD.exe /RD \"%s\"", path));
			else
				DoSlim(path);
		}
		else
		{
			if (!_stricmp(getLocal(path), "GitRelease.bat"))
				coExecute_x(xcout("ren \"%s\" GitRelease.bat_", path));
		}
	}
	releaseDim(paths, 1);
}
int main(int argc, char **argv)
{
	char *dir;

	errorCase(!argIs("ANX-CS-KIT-SLIM")); // ˆÀ‘S‚Ì‚½‚ß

	dir = nextArg();
	dir = makeFullPath(dir);
	errorCase(!existDir(dir));

	DoSlim(dir);

	memFree(dir);
}
