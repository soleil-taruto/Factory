/*
	MergeRum.exe [/F] マスターDIR スレーブDIR
*/

#include "C:\Factory\Common\all.h"

#define EXT_RUM "rum"

#define DIR_FILES "files"
#define DIR_REVISIONS "revisions"

#define FILE_COMMENT "comment.txt"
#define FILE_FILES "files.txt"
#define FILE_TREE "tree.txt"

static int IgnoreDuplRev;

static autoList_t *GetPaths(char *rootDir, char *subDir, int mode)
{
	autoList_t *paths;

	cout("1> %s\n", rootDir);
	cout("2> %s\n", subDir);

	addCwd(rootDir);
	addCwd(subDir);

	paths = (mode == 'F' ? lsFiles : lsDirs)(".");
	cout("%u object(s)\n", getCount(paths));
	eraseParents(paths);

	unaddCwd();
	unaddCwd();

	return paths;
}
static void PathsFltr(autoList_t *paths, char *format)
{
	char *path;
	uint index;

	LOGPOS();

	foreach (paths, path, index)
	{
		toLowerLine(path);
		cout("%s\n", path);
		errorCase(!lineExp(format, path));
	}
	LOGPOS();
}
static void MergeRum(char *masterDir, char *slaveDir)
{
	autoList_t *masterFiles = GetPaths(masterDir, DIR_FILES, 'F');
	autoList_t *masterRevs = GetPaths(masterDir, DIR_REVISIONS, 'D');
	autoList_t *slaveFiles = GetPaths(slaveDir, DIR_FILES, 'F');
	autoList_t *slaveRevs = GetPaths(slaveDir, DIR_REVISIONS, 'D');
	autoList_t *duplFiles;
	autoList_t *duplRevs;

	PathsFltr(masterFiles, "<32,09af>");
	PathsFltr(slaveFiles, "<32,09af>");
	PathsFltr(masterRevs, "<14,09>");
	PathsFltr(slaveRevs, "<14,09>");

	duplFiles = mergeLines(masterFiles, slaveFiles);
	duplRevs = mergeLines(masterRevs, slaveRevs);

	cout("F: %u %u %u\n", getCount(masterFiles), getCount(slaveFiles), getCount(duplFiles));
	cout("R: %u %u %u\n", getCount(masterRevs), getCount(slaveRevs), getCount(duplRevs));

	errorCase_m(!IgnoreDuplRev && getCount(duplRevs), "同じリビジョンがあります。");

	// move rev
	{
		char *rev;
		uint index;

		foreach (slaveRevs, rev, index)
		{
			char *sRev = combine_xc(combine(slaveDir, DIR_REVISIONS), rev);
			char *mRev = combine_xc(combine(masterDir, DIR_REVISIONS), rev);

			cout("move rev\n");
			cout("< %s\n", sRev);
			cout("> %s\n", mRev);

			createDir(mRev);
			moveDir(sRev, mRev);
			removeDir(sRev);

			memFree(sRev);
			memFree(mRev);
		}
	}

	// move file
	{
		char *file;
		uint index;

		foreach (slaveFiles, file, index)
		{
			char *sFile = combine_xc(combine(slaveDir, DIR_FILES), file);
			char *mFile = combine_xc(combine(masterDir, DIR_FILES), file);

			cout("move file\n");
			cout("< %s\n", sFile);
			cout("> %s\n", mFile);

			moveFile(sFile, mFile);

			memFree(sFile);
			memFree(mFile);
		}
	}

	// remove slave
	{
		char *wDir = makeFreeDir();
		char *file;
		char *rev;
		uint index;

		foreach (duplFiles, file, index)
		{
			char *sFile = combine_xc(combine(slaveDir, DIR_FILES), file);
			char *wFile = combine(wDir, file);

			cout("delete file\n");
			cout("< %s\n", sFile);
			cout("> %s\n", wFile);

			moveFile(sFile, wFile);

			memFree(sFile);
			memFree(wFile);
		}
		foreach (duplRevs, rev, index)
		{
			char *s_dir = combine_xc(combine(slaveDir, DIR_REVISIONS), rev);
			char *w_dir = combine(wDir, rev);

			cout("delete dir\n");
			cout("< %s\n", s_dir);
			cout("> %s\n", w_dir);

			movePath(s_dir, w_dir);

			memFree(s_dir);
			memFree(w_dir);
		}
		memFree(wDir);

		LOGPOS();

		{
			char *filesDir = combine(slaveDir, DIR_FILES);
			char *revsDir = combine(slaveDir, DIR_REVISIONS);

			removeDir(filesDir);
			removeDir(revsDir);
			removeDir(slaveDir);

			memFree(filesDir);
			memFree(revsDir);
		}
	}

	LOGPOS();

	releaseDim(masterFiles, 1);
	releaseDim(masterRevs, 1);
	releaseDim(slaveFiles, 1);
	releaseDim(slaveRevs, 1);
	releaseDim(duplFiles, 1);
	releaseDim(duplRevs, 1);

	LOGPOS();
}
int main(int argc, char **argv)
{
	char *masterDir;
	char *slaveDir;

readArgs:
	if (argIs("/F"))
	{
		IgnoreDuplRev = 1;
		goto readArgs;
	}

	masterDir = nextArg();
	slaveDir = nextArg();

	errorCase(m_isEmpty(masterDir));
	errorCase(m_isEmpty(slaveDir));

	masterDir = makeFullPath(masterDir);
	slaveDir = makeFullPath(slaveDir);

	errorCase(!existDir(masterDir));
	errorCase(!existDir(slaveDir));

	errorCase(_stricmp(getExt(masterDir), EXT_RUM));
	errorCase(_stricmp(getExt(slaveDir), EXT_RUM));

	errorCase(!mbs_stricmp(masterDir, slaveDir)); // ? 同じDIR

	cout("MergeRum\n");
	cout("[マスター] > %s\n", masterDir);
	cout("[スレーブ] < %s\n", slaveDir);

	if (IgnoreDuplRev)
		cout("*** 重複しているリビジョンはスレーブ側を破棄する ***\n");

	cout("続行？\n");

	if (getKey() == 0x1b)
		termination(0);

	cout("続行します。\n");

	MergeRum(masterDir, slaveDir);

	cout("\\e\n");
}
