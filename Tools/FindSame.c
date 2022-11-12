/*
	FindSame.exe [指定ファイル [検索対象DIR]]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\md5Cache.h"

static void FindSame(char *targFile, char *searchDir)
{
	autoList_t *foundFiles = newList();
	autoList_t *files;
	char *file;
	uint index;
	uint64 targSize;
	char *targHash;

	targFile = makeFullPath(targFile);
	searchDir = makeFullPath(searchDir);

	errorCase(!existFile(targFile));
	errorCase(!existDir(searchDir));

	files = lssFiles(searchDir);
	targSize = getFileSize(targFile);
	targHash = md5Cache_makeHexHashFile(targFile);

	foreach (files, file, index)
	{
		uint64 size = getFileSize(file);

		if (size == targSize)
		{
			char *hash = md5Cache_makeHexHashFile(file);

			if (!_stricmp(hash, targHash)) // ? found
			{
				cout("%s\n", file);
				addElement(foundFiles, (uint)strx(file));
			}
			memFree(hash);
		}
	}
	if (getCount(foundFiles))
	{
		writeLines(FOUNDLISTFILE, foundFiles);
	}

	cout("見つかった件数：%u\n", getCount(foundFiles));

	releaseDim(foundFiles, 1);
	releaseDim(files, 1);
	memFree(targHash);
	memFree(targFile);
	memFree(searchDir);
}
int main(int argc, char **argv)
{
	if (hasArgs(2))
	{
		FindSame(getArg(0), getArg(1));
	}
	else if (hasArgs(1))
	{
		FindSame(getArg(0), c_dropDir());
	}
	else
	{
		char *file;
		char *dir;

		cout("目的のファイル\n");
		file = dropFile();
		cout("検索先ディレクトリ\n");
		dir  = dropDir();

		FindSame(file, dir);

		memFree(file);
		memFree(dir);
	}
}
