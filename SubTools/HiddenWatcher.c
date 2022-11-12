#include "C:\Factory\Common\all.h"

#define R_DIR "S:\\_hidden"

static void GR_MoveTo(char *repositoryName)
{
	char *wDir = combine("C:\\huge\\GitHub", repositoryName);
	autoList_t *paths;
	char *path;
	uint index;
	uint dirCount;

	LOGPOS();
	errorCase(!existDir(wDir));

	// ---- 古いファイルを削除 ----

	LOGPOS();
	paths = ls(wDir);
	LOGPOS();
	dirCount = lastDirCount;

	foreach (paths, path, index)
	{
		cout("* %s\n", path);

		if (getLocal(path)[0] == '.') // ? .git など
		{
			LOGPOS(); // noop
		}
		else
		{
			if (index < dirCount)
			{
				LOGPOS();
				recurRemoveDir(path);
				LOGPOS();
			}
			else
			{
				LOGPOS();
				removeFile(path);
				LOGPOS();
			}
		}
	}
	LOGPOS();
	releaseDim(paths, 1);

	// ---- 新しいファイルを追加 ----

	LOGPOS();
	paths = ls(".");
	LOGPOS();
	dirCount = lastDirCount;

	foreach (paths, path, index)
	{
		cout("< %s\n", path);

		if (getLocal(path)[0] == '.') // ? .git など？
		{
			LOGPOS(); // noop
		}
		else
		{
			char *wPath = combine(wDir, getLocal(path));

			cout("> %s\n", wPath);

			if (index < dirCount)
			{
				LOGPOS();
				createDir(wPath);
				LOGPOS();
				moveDir(path, wPath);
				LOGPOS();
			}
			else
			{
				LOGPOS();
				moveFile(path, wPath);
				LOGPOS();
			}
			memFree(wPath);
		}
	}
	LOGPOS();
	releaseDim(paths, 1);

	// ----

	memFree(wDir);
	LOGPOS();
}
static void GitRelease(char *rFile, char *repositoryName)
{
	char *midDir = makeTempDir(NULL);

	LOGPOS();

	addCwd(midDir);
	{
		moveFile(rFile, "1.clu");

		coExecute("C:\\Factory\\Tools\\Cluster.exe /R 1.clu 2");

		addCwd("2");
		{
			GR_MoveTo(repositoryName);
		}
		unaddCwd();
	}
	unaddCwd();

	LOGPOS();
	recurRemoveDir_x(midDir);
	LOGPOS();
}
static void Mochikaeri(char *rFile, char *wDir)
{
	char *midDir;

	LOGPOS();
	errorCase(!existDir(wDir));

	midDir = makeTempDir(NULL);

	addCwd(midDir);
	{
		moveFile(rFile, "1.clu");

		coExecute("C:\\Factory\\Tools\\Cluster.exe /R 1.clu 2");

		errorCase(!existDir("2"));

		LOGPOS();
		recurClearDir(wDir);
		LOGPOS();
		moveDir("2", wDir);
		LOGPOS();
	}
	unaddCwd();

	LOGPOS();
	recurRemoveDir_x(midDir);
	LOGPOS();
}
static void Main2(void)
{
	autoList_t *files;
	char *file;
	uint index;

	LOGPOS();
	errorCase(!existDir(R_DIR));

	files =lsFiles(R_DIR);

	sortJLinesICase(files);

	foreach (files, file, index)
	{
		char *ext = getExt(file);

		cout("file: %s\n", file);

		if (!_stricmp(ext, "_gitrel_Denebola"))
		{
			GitRelease(file, "Denebola");
		}
		/*
		else if (!_stricmp(ext, "_gitrel_Spica02"))
		{
			GitRelease(file, "Spica02");
		}
		*/
		else if (!_stricmp(ext, "_mochi_Spica02"))
		{
			Mochikaeri(file, "C:\\pleiades\\workspace\\Spica02\\src");
		}
		else
		{
			cout("SKIP!\n");
		}
	}
	releaseDim(files, 1);
	LOGPOS();
}
int main(int argc, char **argv)
{
	errorCase(strcmp(nextArg(), "S_HIDDEN")); // safety

	Main2();
}
