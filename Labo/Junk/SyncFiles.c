/*
	使い方
		例えば dir1, dir2, dir3 というディレクトリがあって、それぞれの直下にある共通するファイルを同期したい場合、
		dirs.txt に
			dir1
			dir2
			dir3
		と記述して SyncFiles.exe /F dirs.txt で同期する。
		SyncFiles.exe dir1 dir2 dir3 でも同じ。

		各フォルダ内の同名ファイルの中で唯一異なるファイルを更新と見なし、これを他のフォルダに上書きする。
		同時に複数修正してしまった場合、基本的にはコンフリクトになるが、
		全く同じ更新をしなかったファイルが１つの場合、そのファイルが更新と見なされてしまう。

		必ずコンフリクトするように対策をとるなら、
		Common, Mirror ディレクトリを作成して、同期するファイルをそれぞれにコピーして、
		dirs.txt を
			Common
			Mirror
			dir1
			dir2
			dir3
		として SyncFiles.exe /F dirs.txt を実行する。
		同時に複数修正してしまっても Common, Mirror を触らなければ、必ずコンフリクトになる。

		ファイルの追加・削除もするなら SyncFiles.exe /C /D /F dirs.txt と実行する。
		追加・削除は dirs.txt の最初の行に記述されたディレクトリを基準とする。この場合 Common となる。
		/C による追加は Common 内のファイルを他のディレクトリにコピーして Common にしかないファイルが配られる。
		/D による削除は Common 内のファイル一覧から削除するファイルを選択して、削除する。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

static autoList_t *GetFiles(char *dir)
{
	autoList_t *paths = ls(dir);
	autoList_t *files = newList();
	char *file;
	uint index;

	for (index = lastDirCount; index < getCount(paths); index++)
	{
		file = (char *)getElement(paths, index);
		file = strx(getLocal(file));
		addElement(files, (uint)file);
	}
	return files;
}
static autoList_t *GetSharedFiles(autoList_t *dirs)
{
	autoList_t *files = GetFiles((char *)getElement(dirs, 0));
	uint index;

	for (index = 1; index < getCount(dirs); index++)
	{
		autoList_t *nextFiles = GetFiles((char *)getElement(dirs, index));
		autoList_t *commFiles;

		commFiles = merge(files, nextFiles, (sint (*)(uint, uint))mbs_stricmp, (void (*)(uint))memFree);

		releaseDim(files, 1);
		releaseDim(nextFiles, 1);

		files = commFiles;
	}
	return files;
}

static uint Conflicted;

static uint GetModify(autoList_t *hashes) // ret == getCount(hashes): (conflict || not modified)
{
	char *baseHash;
	uint index;
	uint trlidx;

	errorCase(getCount(hashes) < 3);
	Conflicted = 0;

	for (index = 1; index < getCount(hashes); index++)
	{
		if (_stricmp(getLine(hashes, 0), getLine(hashes, index))) // ? 不一致
		{
			goto found_mod;
		}
	}
	goto endfunc;

found_mod:
	trlidx = index;

	baseHash = (char *)getElement(
		hashes,
		index == 1 && !_stricmp(getLine(hashes, 1), getLine(hashes, 2)) ? (index = 0, 1) : 0
		);

	while (
		trlidx++,
		trlidx < getCount(hashes)
		)
	{
		if (_stricmp(baseHash, getLine(hashes, trlidx))) // ? 不一致
		{
			goto found_mod2nd;
		}
	}
	goto endfunc;

found_mod2nd:
	Conflicted = 1;
	index = getCount(hashes);

endfunc:
	return index;
}
static void Synchronize(autoList_t *files, uint modindex)
{
	char *modfile = (char *)getElement(files, modindex);
	char *file;
	uint index;

	foreach (files, file, index)
	{
		if (index != modindex)
		{
			cout("< %s\n", modfile);
			cout("> %s\n", file);

			copyFile(modfile, file);
		}
	}
}

static uint SFModCount;

static void SyncFile(autoList_t *dirs, char *sharedFile)
{
	autoList_t *files = newList();
	autoList_t *hashes = newList();
	autoBlock_t *pab;
	char *file;
	char *hash;
	char *dir;
	uint index;
	uint modindex;

	foreach (dirs, dir, index)
	{
		file = combine(dir, sharedFile);
		hash = makeHexLine(pab = md5_makeHashFile(file));
		releaseAutoBlock(pab);

		cout("* %s %s\n", hash, file);

		addElement(files, (uint)file);
		addElement(hashes, (uint)hash);
	}
	modindex = GetModify(hashes);

	if (modindex < getCount(hashes))
	{
		cout("modify: [%u]\n", modindex);
		Synchronize(files, modindex);
		SFModCount++;
	}
	else if (Conflicted)
	{
		cout("conflict!\n");
		error();
	}
	else
	{
		cout("not modified\n");
	}
	cout("\n");

	releaseDim(files, 1);
	releaseDim(hashes, 1);
}

static uint SFAutoCopy;
static uint SFAutoDelete;

static void SyncFiles(autoList_t *dirs)
{
	autoList_t *files;
	char *file;
	char *dir;
	uint index;

	errorCase(getCount(dirs) < 3);

	foreach (dirs, dir, index)
	{
		cout("%s\n", dir);
		errorCase(!existDir(dir));
	}
	errorCase(findPair(dirs, (sint (*)(uint, uint))mbs_stricmp));
	cout("\n");

	files = GetSharedFiles(dirs);

	SFModCount = 0;
	foreach (files, file, index)
	{
		SyncFile(dirs, file);
	}
	cout("%u file(s) synchronized.\n", SFModCount);

	if (SFAutoCopy)
	{
		cout("\n");

		for (index = 1; index < getCount(dirs); index++)
		{
			char *dir1 = getLine(dirs, 0);
			char *dir2 = getLine(dirs, index);

			cout("%s > %s\n", dir1, dir2);
			copyDir(dir1, dir2); // HACK: ディレクトリがあると多分落ちる。
		}
		cout("auto copy ok.\n");
	}
	if (SFAutoDelete)
	{
		cout("\n");
		cout("if you want to delete some files, press 'D' key while 3 seconds...\n");

		if (waitKey(3000) == 'D')
		{
			autoList_t *files = lsFiles(getLine(dirs, 0));
			autoList_t *delFiles;

			eraseParents(files);
			delFiles = selectLines(files);

			foreach (dirs, dir, index)
			{
				uint fIndex;

				addCwd(dir);

				foreach (delFiles, file, fIndex)
					removeFile(file);

				unaddCwd();
			}
			releaseDim(files, 1);
			releaseDim(delFiles, 1);

			cout("auto delete ok.\n");
		}
		else
		{
			cout("auto delete cancelled.\n");
		}
	}
}

int main(int argc, char **argv)
{
	autoList_t *dirs = newList();

readArgs:
	if (argIs("/C")) // auto Copy
	{
		SFAutoCopy = 1;
		goto readArgs;
	}
	if (argIs("/D")) // auto Delete
	{
		SFAutoDelete = 1;
		goto readArgs;
	}
	if (argIs("/F")) // dirs from File
	{
		autoList_t *lines = readResourceLines(nextArg());
		char *dir;
		uint index;

		foreach (lines, dir, index)
		{
			addElement(dirs, (uint)makeFullPath(dir));
		}
		goto readArgs;
	}

	while (hasArgs(1))
	{
		addElement(dirs, (uint)makeFullPath(nextArg()));
	}
	SyncFiles(dirs);
}
