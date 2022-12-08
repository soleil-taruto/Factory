/*
	BatToriai.exe TARGET-DIR [/L | /S]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\FLock.h"

#define LOCK_FILE "BatToriai_{45d85873-205e-47a3-a14d-e3a7df8f2b37}.tmp"

#define EXT_UNDONE "bat_undone"
#define EXT_BAT "bat"
#define EXT_DONE "bat_done"
#define EXT_ZZZ "bat_zzz"

static char *GetNextBatFile(void)
{
	FILE *lFp;
	char *batFile;

	lFp = FLockLoop(LOCK_FILE);
	{
		autoList_t *files = lsFiles(".");
		char *file;
		uint index;

		sortJLinesICase(files);

		foreach (files, file, index)
			if (!_stricmp(EXT_UNDONE, getExt(file)))
				break;

		if (file)
		{
			batFile = changeExt(getLocal(file), EXT_BAT);
			moveFile(file, batFile);
		}
		else
		{
			batFile = NULL;
		}
		releaseDim(files, 1);
	}
	FUnlock(lFp);

	return batFile;
}
static void BatFile_Done(char *batFile)
{
	FILE *lFp;

	lFp = FLockLoop(LOCK_FILE);
	{
		char *doneFile = changeExt(batFile, EXT_DONE);

		moveFile(batFile, doneFile);
		memFree(doneFile);
	}
	FUnlock(lFp);
}
static void BatToriai(void)
{
	for (; ; )
	{
		char *batFile = GetNextBatFile();

		if (!batFile)
			break;

		coExecute(batFile);
		BatFile_Done(batFile);
		memFree(batFile);
	}
}
static void BatToriai_Stop(void)
{
	FILE *lFp;

	lFp = FLockLoop(LOCK_FILE);
	{
		autoList_t *files = lsFiles(".");
		char *file;
		uint index;

		sortJLinesICase(files); // 2bs

		foreach (files, file, index)
		{
			if (!_stricmp(EXT_UNDONE, getExt(file)))
			{
				char *zzzFile = changeExt(file, EXT_ZZZ);

				moveFile(file, zzzFile);
				memFree(zzzFile);
			}
		}
		releaseDim(files, 1);
	}
	FUnlock(lFp);
}
int main(int argc, char **argv)
{
	addCwd(nextArg());

	if (argIs("/L"))
	{
		createFile(LOCK_FILE);
	}
	else if (argIs("/S"))
	{
		BatToriai_Stop();
	}
	else
	{
		BatToriai();
	}
	unaddCwd();
}
