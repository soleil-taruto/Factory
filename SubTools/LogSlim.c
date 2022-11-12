/*
	LogSlim.exe [/TSX TOTAL-SIZE-MAX] [/SX DIR-SIZE-MAX] [/CX DIR-FILE-COUNT-MAX] ROOT-DIR "DELETE-OLD-LOG"

		TOTAL-SIZE-MAX     ... ルートディレクトリ配下のファイル・サイズ合計がこれより大きくならないようにする。
		DIR-SIZE-MAX       ... 各ディレクトリの直下のファイル・サイズ合計がこれより大きくならないようにする。
		DIR-FILE-COUNT-MAX ... 各ディレクトリの直下のファイル数がこれより多くならないようにする。
*/

#include "C:\Factory\Common\all.h"

static uint64 TotalSizeMax = 10000000000ui64; // 10 GB
static uint64 DirSizeMax = 1000000000ui64; // 1 GB
static uint DirFileCountMax = 1000;

static char *RootDir;

typedef struct FileInfo_st
{
	char *File;
	uint64 Size;
}
FileInfo_t;

typedef struct DirInfo_st
{
	char *Dir;
	autoList_t *FileInfos;
}
DirInfo_t;

static autoList_t *DirInfos;

static FileInfo_t *CreateFileInfo(char *file)
{
	FileInfo_t *i = nb_(FileInfo_t);

	i->File = strx(file);
	i->Size = getFileSize(file);

	return i;
}
static void ReleaseFileInfo(FileInfo_t *i)
{
	memFree(i->File);
	memFree(i);
}
static void AddDirInfo(char *dir)
{
	DirInfo_t *i = nb_(DirInfo_t);

	i->Dir = strx(dir);
	i->FileInfos = newList();

	{
		autoList_t *files = cmdDir_lsFiles(dir);
		uint index;

		sortJLinesICase(files);

		/*
			最後のファイル (最新のファイル) は更新中かもしれないので除外する。
		*/
		for (index = 0; index + 1 < getCount(files); index++)
		{
			addElement(i->FileInfos, (uint)CreateFileInfo(getLine(files, index)));
		}
		releaseDim(files, 1);
	}
	addElement(DirInfos, (uint)i);
}
static void InitDirInfos(void)
{
	autoList_t *dirs;
	char *dir;
	uint index;

	LOGPOS();

	dirs = cmdDir_lssDirs(RootDir);
	DirInfos = newList();
	AddDirInfo(RootDir);

	foreach (dirs, dir, index)
	{
		cout("[%.4f] %s\n", (double)index / getCount(dirs), dir);

		AddDirInfo(dir);
	}
	releaseDim(dirs, 1);

	LOGPOS();
}

// ---- slim ----

static void SlimDir(DirInfo_t *i)
{
	FileInfo_t *fi;
	uint index;
	uint64 size;

	for (index = 0; index + DirFileCountMax < getCount(i->FileInfos); index++)
	{
		LOGPOS();

		fi = (FileInfo_t *)getElement(i->FileInfos, index);

		removeFile(fi->File);

		ReleaseFileInfo(fi);
		setElement(i->FileInfos, index, 0);
	}
	removeZero(i->FileInfos);

	size = 0;

	foreach (i->FileInfos, fi, index)
		size += fi->Size;

	for (index = 0; index < getCount(i->FileInfos) && DirSizeMax < size; index++)
	{
		LOGPOS();

		fi = (FileInfo_t *)getElement(i->FileInfos, index);

		removeFile(fi->File);
		size -= fi->Size;

		ReleaseFileInfo(fi);
		setElement(i->FileInfos, index, 0);
	}
	removeZero(i->FileInfos);
}
static void SlimDirs(void)
{
	DirInfo_t *i;
	uint index;

	foreach (DirInfos, i, index)
	{
		SlimDir(i);
	}
}
static void SlimByTotal(void)
{
	DirInfo_t *i;
	uint index;
	uint64 size;

	size = 0;

	foreach (DirInfos, i, index)
	{
		FileInfo_t *fi;
		uint fi_index;

		foreach (i->FileInfos, fi, fi_index)
			size += fi->Size;
	}
	while (TotalSizeMax < size)
	{
		LOGPOS();

		foreach (DirInfos, i, index)
		{
			if (1 <= getCount(i->FileInfos))
			{
				FileInfo_t *fi = (FileInfo_t *)desertElement(i->FileInfos, 0);

				removeFile(fi->File);
				size -= fi->Size;

				ReleaseFileInfo(fi);
			}
		}
	}
}

// ----

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/TSX"))
	{
		TotalSizeMax = toValue64(nextArg());
		goto readArgs;
	}
	if (argIs("/SX"))
	{
		DirSizeMax = toValue64(nextArg());
		goto readArgs;
	}
	if (argIs("/CX"))
	{
		DirFileCountMax = toValue(nextArg());
		goto readArgs;
	}
	RootDir = makeFullPath(nextArg());

	cout("TotalSizeMax: %I64u\n", TotalSizeMax);
	cout("DirSizeMax: %I64u\n", DirSizeMax);
	cout("DirFileCountMax: %I64u\n", DirFileCountMax);
	cout("RootDir: %s\n", RootDir);

	errorCase(!m_isRange(TotalSizeMax, 1, IMAX_64));
	errorCase(!m_isRange(DirSizeMax, 1, IMAX_64));
	errorCase(!m_isRange(DirFileCountMax, 1, IMAX));

	errorCase(!existDir(RootDir));

	errorCase(_stricmp("DELETE-OLD-LOG", nextArg())); // 安全のため

	InitDirInfos();

	SlimDirs();
	SlimByTotal();
}
