/*
	DSK_BOTH_* ‚Ì‚Æ‚« DirSabun_t.Path ‚É‚Í lDir ‚ÌƒpƒX‚ª“ü‚éB
*/

#include "all.h"

DirSabun_t *CreateDirSabun(char *path, DirSabunKind_t kind, autoList_t *lDiff, autoList_t *rDiff)
{
	DirSabun_t *i = nb_(DirSabun_t);

	i->Path = path; // bind
	i->Kind = kind;
	i->LDiff = lDiff; // bind, NULL ok
	i->RDiff = rDiff; // bind, NULL ok

	return i;
}
void ReleaseDirSabun(DirSabun_t *i)
{
	if (!i)
		return;

	memFree(i->Path);
	ReleaseSabunList(i->LDiff);
	ReleaseSabunList(i->RDiff);
	memFree(i);
}
void ReleaseDirSabunList(autoList_t *list)
{
	if (!list)
		return;

	callAllElement(list, (void (*)(uint))ReleaseDirSabun);
	releaseAutoList(list);
}

static void AddBothExist(autoList_t *dest, autoList_t *paths, char *lParent, char *rParent, int lWithData, int rWithData)
{
	char *path;
	uint index;

	foreach (paths, path, index)
	{
		char *lPath = combine(lParent, path);
		char *rPath = combine(rParent, path);
		int lIsDir;
		int rIsDir;

		lIsDir = existDir(lPath);
		rIsDir = existDir(rPath);

		if (lIsDir ? !rIsDir : rIsDir) // ? lIsDir != rIsDir
		{
			if (lIsDir)
			{
				addElement(dest, (uint)CreateDirSabun(strx(path), DSK_LONLY_DIR, NULL, NULL));
				addElement(dest, (uint)CreateDirSabun(strx(path), DSK_RONLY_FILE, NULL, NULL));
			}
			else
			{
				addElement(dest, (uint)CreateDirSabun(strx(path), DSK_LONLY_FILE, NULL, NULL));
				addElement(dest, (uint)CreateDirSabun(strx(path), DSK_RONLY_DIR, NULL, NULL));
			}
		}
		else
		{
			if (lIsDir)
			{
				addElement(dest, (uint)CreateDirSabun(strx(path), DSK_BOTH_DIR, NULL, NULL));
			}
			else
			{
				autoBlock_t *lFileData = readBinary(lPath);
				autoBlock_t *rFileData = readBinary(rPath);
				autoList_t *lDiff = newList();
				autoList_t *rDiff = newList();

				MakeSabun(lFileData, rFileData, lDiff, rDiff, lWithData, rWithData);

				addElement(dest, (uint)CreateDirSabun(strx(path), DSK_BOTH_FILE, lDiff, rDiff));

				releaseAutoBlock(lFileData);
				releaseAutoBlock(rFileData);
			}
		}
		memFree(lPath);
		memFree(rPath);
	}
}
static void AddOnlyExist(autoList_t *dest, autoList_t *paths, char *parent, int isLeft)
{
	char *path;
	uint index;

	addCwd(parent);

	foreach (paths, path, index)
	{
		DirSabunKind_t kind;

		if (existDir(path))
			kind = isLeft ? DSK_LONLY_DIR : DSK_RONLY_DIR;
		else
			kind = isLeft ? DSK_LONLY_FILE : DSK_RONLY_FILE;

		addElement(dest, (uint)CreateDirSabun(strx(path), kind, NULL, NULL));
	}
	unaddCwd();
}
autoList_t *MakeDirSabun(char *lDir, char *rDir, int lWithData, int rWithData)
{
	autoList_t *lPaths = lss(lDir);
	autoList_t *rPaths = lss(rDir);
	autoList_t *mPaths;
	autoList_t *result = newList();

	changeRootsAbs(lPaths, lDir, NULL);
	changeRootsAbs(rPaths, rDir, NULL);

	mPaths = merge(lPaths, rPaths, (sint (*)(uint, uint))mbs_stricmp, (void (*)(uint))memFree);

	AddBothExist(result, mPaths, lDir, rDir, lWithData, rWithData);
	AddOnlyExist(result, lPaths, lDir, 1);
	AddOnlyExist(result, rPaths, rDir, 0);

	releaseDim(lPaths, 1);
	releaseDim(rPaths, 1);
	releaseDim(mPaths, 1);

	return result;
}
