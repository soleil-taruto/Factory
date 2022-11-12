#include "GitSourceMask.h"

#define FLAG_FILE      "_gitsrcmsk"
#define FLAG_TXT_FILE  "_gittxtmsk"
#define MSK_FILES_FILE "_gitsrcmsk_files"
#define IGN_FILES_FILE "_gitignore_files"
#define IGN_DIRS_FILE  "_gitignore_dirs"

static void MskSrcFile(char *file, int eurpFlag)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;
	char *p;

	LOGPOS();

	foreach (lines, line, index)
	{
		for (p = line; *p; p++)
			if (' ' < *p)
				*p = '/';

		p = strchr(line, '/');

		if (p && p[1] != '/')
		{
			if (p[1])
				p[1] = '/';
			else
				setElement(lines, index, (uint)addChar(line, '/'));
		}
	}
	p = md5_makeHexHashFile(file);
	insertElement(lines, 0, (uint)xcout("//// _git source mask md5:%s ////", p));
	memFree(p);

	writeLines_cx(file, lines);

	LOGPOS();

	if (eurpFlag)
		EscapeUnusableResPath(file);

	PostGitMaskFile(file);
}
static void MaskSourceFile(char *file)
{
	char *ext = getExt(file);

	cout("* %s\n", file);

	     if (!_stricmp(ext, "c"    )) MskSrcFile(file, 0);
	else if (!_stricmp(ext, "h"    )) MskSrcFile(file, 0);
	else if (!_stricmp(ext, "cs"   )) MskSrcFile(file, 0);
	else if (!_stricmp(ext, "cpp"  )) MskSrcFile(file, 0);
	else if (!_stricmp(ext, "java" )) MskSrcFile(file, 0);
}
static void MaskTextFile(char *file)
{
	char *ext = getExt(file);

	cout("# %s\n", file);

	     if (!_stricmp(ext, "txt")) MskSrcFile(file, 1);
	else if (!_stricmp(ext, "csv")) MskSrcFile(file, 1);
}
static void MaskSourceByResFile(autoList_t *files)
{
	char *file;
	uint index;

	LOGPOS();

	foreach (files, file, index)
	{
		cout("* %s\n", file);

		if (!existFile(file))
		{
			cout("¡¡¡íœÏ‚Ý¡¡¡\n"); // IGN_FILES_FILE, IGN_DIRS_FILE ‚É‚æ‚èAŠù‚Éíœ‚³‚ê‚Ä‚¢‚é‰Â”\«‚à‚ ‚éB
		}
		else if (!_stricmp(MSK_FILES_FILE, getLocal(file)))
		{
			autoList_t *mskfiles = readResourceLines(file);
			char *mskfile;
			uint mskfile_index;

			LOGPOS();

			foreach (mskfiles, mskfile, mskfile_index)
			{
				cout("** %s\n", mskfile);

				errorCase(!isFairRelPath(mskfile, 0));
				mskfile = changeLocal(file, mskfile);
				errorCase(!existFile(mskfile));

				MskSrcFile(mskfile, 1);

				memFree(mskfile);
			}
			releaseDim(mskfiles, 1);

			LOGPOS();
		}
		else if (!_stricmp(IGN_FILES_FILE, getLocal(file)))
		{
			autoList_t *ignfiles = readResourceLines(file);
			char *ignfile;
			uint ignfile_index;

			LOGPOS();

			foreach (ignfiles, ignfile, ignfile_index)
			{
				cout("*!F %s\n", ignfile);

				errorCase(!isFairRelPath(ignfile, 0));
				ignfile = changeLocal(file, ignfile);
				errorCase(!existFile(ignfile));

				removeFile(ignfile);
				PostGitIgnoreFile(ignfile);

				memFree(ignfile);
			}
			releaseDim(ignfiles, 1);

			LOGPOS();
		}
		else if (!_stricmp(IGN_DIRS_FILE, getLocal(file)))
		{
			autoList_t *igndirs = readResourceLines(file);
			char *igndir;
			uint igndir_index;

			LOGPOS();

			foreach (igndirs, igndir, igndir_index)
			{
				cout("*!D %s\n", igndir);

				errorCase(!isFairRelPath(igndir, 0));
				igndir = changeLocal(file, igndir);
				errorCase(!existDir(igndir));

				recurClearDir(igndir);

				{
					char *dmyfile = combine(igndir, "*");

					PostGitIgnoreFile(dmyfile);
					memFree(dmyfile);
				}

				memFree(igndir);
			}
			releaseDim(igndirs, 1);

			LOGPOS();
		}
	}
	LOGPOS();
}
static void GitSourceMask_Sub(autoList_t *files, char *flagFile, void (*maskFunc)(char *))
{
	char *file;
	uint index;
	autoList_t *targets = newList();

	foreach (files, file, index)
	{
		if (!_stricmp(flagFile, getLocal(file)))
		{
			char *prefix = addChar(changeLocal(file, ""), '\\');
			char *subfile;
			uint subfile_index;

			foreach (files, subfile, subfile_index)
				if (startsWithICase(subfile, prefix))
					addElement(targets, (uint)subfile);

			memFree(prefix);

			distinct2(targets, simpleComp, noop_u);
		}
	}
	foreach (targets, file, index)
	{
		maskFunc(file);
	}
	releaseAutoList(targets);
}
void GitSourceMask(char *rootDir)
{
	autoList_t *files = lssFiles(rootDir);

	LOGPOS();

	RemoveGitPaths(files);

	GitSourceMask_Sub(files, FLAG_FILE, MaskSourceFile);
	GitSourceMask_Sub(files, FLAG_TXT_FILE, MaskTextFile);

	MaskSourceByResFile(files);

	releaseDim(files, 1);

	LOGPOS();
}
