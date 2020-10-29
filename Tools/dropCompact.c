/*
	dropCompact.exe [/U]
*/

#include "C:\Factory\Common\all.h"

//#define COMPACT_FILE_SIZE_MAX 1000000ui64 // 1MB -- test
#define COMPACT_FILE_SIZE_MAX 10000000000ui64 // 10GB

static int IsUncompactExt_File(char *file)
{
	char *ext = getExt(file);

	return
		!_stricmp(ext, "gz") ||
		!_stricmp(ext, "zip");
}
static void CompactDirHasHugeFile(char *dir)
{
	autoList_t *paths = lss(dir);
	char *path;
	uint index;

	addElement(paths, (uint)strx(dir));

	foreach(paths, path, index)
	{
		cmdTitle_x(xcout("dropCompact - %u / %u", index, getCount(paths)));

		if(existFile(path) && (IsUncompactExt_File(path) || COMPACT_FILE_SIZE_MAX < getFileSize(path)))
		{
			coExecute_x(xcout("Compact.exe /U \"%s\"", path));
		}
		else
		{
			coExecute_x(xcout("Compact.exe /C \"%s\"", path));
		}
	}
	releaseDim(paths, 1);

	cmdTitle("dropCompact");
}
static int HasHugeFile(char *dir)
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;
	int ret = 0;

	foreach(files, file, index)
	{
		cout("* %s\n", file);

		if(IsUncompactExt_File(file) || COMPACT_FILE_SIZE_MAX < getFileSize(file))
		{
			cout("�񈳏k�I\n");
			ret = 1;
			break;
		}
	}
	releaseDim(files, 1);
	return ret;
}
int main(int argc, char **argv)
{
	char *path;

	if(argIs("/U"))
	{
		for(; ; )
		{
			cout("+----------+\n");
			cout("| ���k���� |\n");
			cout("+----------+\n");

			path = dropDirFile();

			if(existDir(path))
			{
				coExecute_x(xcout("Compact.exe /U /S:\"%s\"", path));
			}
			else // file
			{
				coExecute_x(xcout("Compact.exe /U \"%s\"", path));
			}
			cout("\n");
		}
	}

	for(; ; )
	{
		cout("+------+\n");
		cout("| ���k |\n");
		cout("+------+\n");

		path = dropDirFile();

		if(existDir(path))
		{
			if(HasHugeFile(path))
			{
				CompactDirHasHugeFile(path);
			}
			else
			{
				coExecute_x(xcout("Compact.exe /C /S:\"%s\"", path));
			}
		}
		else // file
		{
			coExecute_x(xcout("Compact.exe /C \"%s\"", path));
		}
		cout("\n");
	}
}
