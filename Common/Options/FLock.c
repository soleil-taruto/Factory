#include "FLock.h"

FILE *FLockLoop(char *file)
{
	uint millis = 2000;

	LOGPOS();

	for (; ; )
	{
		FILE *fp = FLock(file);

		if (fp)
		{
			LOGPOS();
			return fp;
		}
		coSleep(millis + getCryptoRand16() % 2000);
		millis = m_min(millis + 500, 5000);
	}
}
FILE *FLock(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	int fd;
	HANDLE fh;

	fd = _fileno(fp);
	fh = (HANDLE)_get_osfhandle(fd);

	if (!LockFile(fh, 0, 0, UINTMAX, UINTMAX)) // ? ロック失敗
	{
		fileClose(fp);
		return NULL;
	}
	return fp;
}
void FUnlock(FILE *fp)
{
	int fd = _fileno(fp);
	HANDLE fh;

	fh = (HANDLE)_get_osfhandle(fd);
	errorCase(!UnlockFile(fh, 0, 0, UINTMAX, UINTMAX)); // ? ロック解除失敗
	fileClose(fp);
}
