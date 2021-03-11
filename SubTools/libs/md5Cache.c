#include "md5Cache.h"

#define CACHE_DIR "C:\\Factory\\tmp\\md5Cache"

static char *GSC_FS_Hash;

static int GSC_FS_Action(struct _finddata_t *i)
{
	char *sHash;

	if(i->name[0] == '.') // ? "." or ".."
		return 1;

	sHash = strx(i->name);
	toLowerLine(sHash);

	errorCase(!lineExp("<32,09af>", sHash));

	GSC_FS_Hash = sHash;

	return 0; // åpë±ÇµÇ»Ç¢ÅB
}
static char *GetOrSetCache(char *sHFile, char *sHInfo, char *sHash)
{
	char *dir1 = combine(CACHE_DIR, sHFile);
	char *dir2;

	dir2 = combine(dir1, sHInfo);

	if(sHash) // Set
	{
		char *symFile = combine(dir2, sHash);

		createDirIfNotExist(CACHE_DIR);
		recurRemoveDirIfExist(dir1);
		createDir(dir1);
		createDir(dir2);
		createFile(symFile);

		memFree(symFile);
	}
	else // Get
	{
		char *wCard = xcout("%s\\*", dir2);

		GSC_FS_Hash = NULL;
		fileSearch(wCard, GSC_FS_Action);
		sHash = GSC_FS_Hash;

		memFree(wCard);
	}
	memFree(dir1);
	memFree(dir2);
	return sHash;
}
char *md5Cache_makeHexHashFile(char *file)
{
	char *sHFile;
	char *sInfo;
	char *sHInfo;
	char *sHash;

	errorCase(!existFile(file));

	file = makeFullPath(file);
	toUpperLine(file);
	sHFile = md5_makeHexHashLine(file);
	sInfo = xcout("%I64u:%I64d:%I64d", getFileSize(file), getFileCreateTime(file), getFileWriteTime(file));
	sHInfo = md5_makeHexHashLine(sInfo);
	sHash = GetOrSetCache(sHFile, sHInfo, NULL);

	if(!sHash)
	{
		sHash = md5_makeHexHashFile(file);
		GetOrSetCache(sHFile, sHInfo, sHash);
	}
	memFree(file);
	memFree(sHFile);
	memFree(sInfo);
	memFree(sHInfo);
	return sHash;
}
autoBlock_t *md5Cache_makeHashFile(char *file)
{
	return makeBlockHexLine_x(md5Cache_makeHexHashFile(file));
}
