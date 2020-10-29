/*
	rumCommitHistory.exe [ROOT_DIR]

		ROOT_DIR ... È—ª‚µ‚½ê‡‚ÍƒJƒŒƒ“ƒgDIR
*/

#include "C:\Factory\Common\all.h"

static autoList_t *Revisions;

static void Search(char *rootDir)
{
	autoList_t *dirs = lsDirs(rootDir);
	char *dir;
	uint index;

	foreach(dirs, dir, index)
	{
		if(!_stricmp("rum", getExt(dir)))
		{
			char *revRootDir = combine(dir, "revisions");
			autoList_t *revDirs;
			char *revDir;
			uint revDir_index;

			revDirs = lsDirs(revRootDir);

			foreach(revDirs, revDir, revDir_index)
			{
				char *commentFile = combine(revDir, "comment.txt");
				char *comment;

				comment = readFirstLine(commentFile);

				addElement(Revisions, (uint)xcout("%s %s > %s", getLocal(revDir), dir, comment));

				memFree(comment);
				memFree(commentFile);
			}
			releaseDim(revDirs, 1);
			memFree(revRootDir);
		}
		else
			Search(dir);
	}
	releaseDim(dirs, 1);
}
static void Main2(char *rootDir)
{
	char *revision;
	uint index;

	Revisions = newList();
	Search(rootDir);
	rapidSortLines(Revisions);

	foreach(Revisions, revision, index)
		cout("%s\n", revision);
}
int main(int argc, char **argv)
{
	if(hasArgs(1))
	{
		Main2(nextArg());
	}
	else
	{
		Main2(".");
	}
}
