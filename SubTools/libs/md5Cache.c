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
static char *GetOrSetCache(char *sHPath, char *sHInfo, char *sHash)
{
	char *dirG0 = GetCacheDir();
	char *dirG1;
	char *dirG2;
	char *dirG3;
	char *dirHP;
	char *dirHI;

	dirG1 = combine_cx(dirG0, strxl(sHPath + 0, 3));
	dirG2 = combine_cx(dirG1, strxl(sHPath + 3, 3));
	dirG3 = combine_cx(dirG2, strxl(sHPath + 6, 3));
	dirHP = combine(dirG3, sHPath);
	dirHI = combine(dirHP, sHInfo);

	if(sHash) // Set
	{
		char *symFile = combine(dirHI, sHash);

		createDirIfNotExist(dirG0);
		createDirIfNotExist(dirG1);
		createDirIfNotExist(dirG2);
		createDirIfNotExist(dirG3);
		recurRemoveDirIfExist(dirHP);
		createDir(dirHP);
		createDir(dirHI);
		createFile(symFile);

		memFree(symFile);
	}
	else if(existDir(dirHI)) // Get
	{
		autoList_t *symFiles = lsFiles(dirHI);

		errorCase(getCount(symFiles) != 1);

		sHash = getLine(symFiles, 0);
		eraseParent(sHash);
		toLowerLine(sHash);

		errorCase(!lineExp("<32,09af>", sHash));

		releaseAutoList(symFiles);
	}
//	memFree(dirG0); // dont!
	memFree(dirG1);
	memFree(dirG2);
	memFree(dirG3);
	memFree(dirHP);
	memFree(dirHI);
	return sHash;
}
char *md5Cache_makeHexHashFile(char *file)
{
	char *sHPath;
	char *sInfo;
	char *sHInfo;
	char *sHash;

	errorCase(!existFile(file));

	file = makeFullPath(file);
	toUpperLine(file);
	sHPath = md5_makeHexHashLine(file);
	sInfo = xcout("%I64u:%I64d:%I64d", getFileSize(file), getFileCreateTime(file), getFileWriteTime(file)); // この情報が変わらなければ、変更無しと見なし、ハッシュ再計算を行わない。
	sHInfo = md5_makeHexHashLine(sInfo);
	sHash = GetOrSetCache(sHPath, sHInfo, NULL);

	if(!sHash)
	{
		sHash = md5_makeHexHashFile(file);
		GetOrSetCache(sHPath, sHInfo, sHash);
	}
	memFree(file);
	memFree(sHPath);
	memFree(sInfo);
	memFree(sHInfo);
	return sHash;
}
autoBlock_t *md5Cache_makeHashFile(char *file)
{
	return makeBlockHexLine_x(md5Cache_makeHexHashFile(file));
}
