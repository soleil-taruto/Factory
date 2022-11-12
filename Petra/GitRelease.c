#include "C:\Factory\Common\all.h"

#define R_ROOT_DIR "C:\\Factory"
#define W_ROOT_DIR "C:\\home\\GitHub\\Factory"

static void ClearRepoDir(char *dir)
{
	autoList_t *paths = ls(dir);
	char *path;
	uint index;

	foreach(paths, path, index)
		if (!startsWithICase(getLocal(path), ".git"))
			recurRemovePath(path);

	releaseDim(paths, 1);
}
static void CopyToRepoDir(char *rDir, char *wDir)
{
	autoList_t *rPaths = ls(rDir);
	char *rPath;
	uint index;

	foreach(rPaths, rPath, index)
	{
		char *wPath = changeRoot(strx(rPath), rDir, wDir);

		cout("< %s\n", rPath);
		cout("> %s\n", wPath);

		copyPath(rPath, wPath);
		memFree(wPath);
	}
	releaseDim(rPaths, 1);
}
static void RemoveNotNeedFiles(char *dir)
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;

	foreach(files, file, index)
	{
		char *ext = getExt(file);

		if (
			!_stricmp(ext, "exe") ||
			!_stricmp(ext, "obj")
			)
			removeFile(file);
	}
	releaseDim(files, 1);

	// ----

	addCwd(dir);
	{
		recurClearDir("tmp");
		recurClearDir("tmp_Prime");
	}
	unaddCwd();
}
int main(int argc, char **argv)
{
	LOGPOS();
	errorCase(!existDir(R_ROOT_DIR));
	errorCase(!existDir(W_ROOT_DIR));

	LOGPOS();
	ClearRepoDir(W_ROOT_DIR);
	LOGPOS();
	CopyToRepoDir(R_ROOT_DIR, W_ROOT_DIR);
	LOGPOS();
	RemoveNotNeedFiles(W_ROOT_DIR);
	LOGPOS();
}
