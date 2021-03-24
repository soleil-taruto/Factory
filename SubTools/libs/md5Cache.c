#include "md5Cache.h"

#define CACHE_UUID "{cb89b2e7-6f61-4a3b-9ccc-39212b231988}"

static char *GetCacheDir(void)
{
	static char *dir;

	if(!dir)
	{
		char *tmpDir = getEnvLine("TMP");

		errorCase(m_isEmpty(tmpDir));
		errorCase(!existDir(tmpDir));

		dir = combine(tmpDir, CACHE_UUID);
//		dir = combine_cx(tmpDir, xcout("%s_%u", CACHE_UUID, (uint)(toValue64_x(makeCompactStamp(NULL)) / 1000000)));
//		dir = combine_cx(tmpDir, xcout("%s_%u", CACHE_UUID, (uint)(time(NULL) / 86400)));
	}
	return dir;
}
static char *GetOrSetCache(char *sHFile, char *sHInfo, char *sHash)
{
	char *dir1 = combine(GetCacheDir(), sHFile);
	char *dir2;

	dir2 = combine(dir1, sHInfo);

	if(sHash) // Set
	{
		char *symFile = combine(dir2, sHash);

		createDirIfNotExist(GetCacheDir());
		recurRemoveDirIfExist(dir1);
		createDir(dir1);
		createDir(dir2);
		createFile(symFile);

		memFree(symFile);
	}
	else if(existDir(dir2)) // Get
	{
		autoList_t *symFiles = lsFiles(dir2);

		errorCase(getCount(symFiles) != 1);

		sHash = getLine(symFiles, 0);
		eraseParent(sHash);
		toLowerLine(sHash);

		errorCase(!lineExp("<32,09af>", sHash));

		releaseAutoList(symFiles);
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
	sInfo = xcout("%I64u:%I64d:%I64d", getFileSize(file), getFileCreateTime(file), getFileWriteTime(file)); // この情報が変わらなければ、変更無しと見なし、ハッシュ再計算を行わない。
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
