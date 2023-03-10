#include "md5Cache.h"

#define CACHE_UUID "{cb89b2e7-6f61-4a3b-9ccc-39212b231988}"

static char *GetCacheDir(void)
{
	static char *dir;

	if (!dir)
	{
		char *tmpDir = "C:\\Factory\\tmp";
//		char *tmpDir = "C:\\tmp";
//		char *tmpDir = "C:\\Factory\\tmp_data";
//		char *tmpDir = getEnvLine("TMP");

//		errorCase(m_isEmpty(tmpDir));
		errorCase(!existDir(tmpDir));

		dir = combine(tmpDir, "md5");
//		dir = combine(tmpDir, CACHE_UUID);
//		dir = combine_cx(tmpDir, xcout("%s_%u", CACHE_UUID, (uint)(toValue64_x(makeCompactStamp(NULL)) / 10000000000))); // 年毎
//		dir = combine_cx(tmpDir, xcout("%s_%u", CACHE_UUID, (uint)(toValue64_x(makeCompactStamp(NULL)) /   100000000))); // 月毎
//		dir = combine_cx(tmpDir, xcout("%s_%u", CACHE_UUID, (uint)(toValue64_x(makeCompactStamp(NULL)) /     1000000))); // 日毎
//		dir = combine_cx(tmpDir, xcout("%s_%u", CACHE_UUID, (uint)(time(NULL) / 86400)));
	}
	return dir;
}
static char *GetOrSetCache_NoLock(char *sHPath, char *sHInfo, char *sHash)
{
	char *dirG0 = GetCacheDir();
	char *dirG1;
	char *dirG2;
	char *dirG3;
	char *dirG4;
	char *dirHP;
	char *dirHI;

	// 定期的なキャッシュクリア
	{
		static int oncePerProc;

		if (!oncePerProc)
		{
			oncePerProc = 1;

			{
				char *EXPIRE_TIME_FILE = combine(dirG0, "____expire.txt");
				char *TARGET_DIR = dirG0;
				time_t PERIOD_SEC = 100 * 86400;
				time_t currTime = time(NULL);
				int cancel = 0;

				if (existFile(EXPIRE_TIME_FILE))
				{
					time_t expireTime = (time_t)toValue64_x(readText_b(EXPIRE_TIME_FILE));

					if (currTime < expireTime)
						cancel = 1;
				}
				if (!cancel)
				{
					time_t nextExpireTime = currTime + PERIOD_SEC;

					LOGPOS();
					recurRemoveDirIfExist(TARGET_DIR);
					createDir(TARGET_DIR);
					writeOneLineNoRet_b_cx(EXPIRE_TIME_FILE, xcout("%I64d", nextExpireTime));
					LOGPOS();
				}
				memFree(EXPIRE_TIME_FILE);
			}
		}
	}

	dirG1 = combine_cx(dirG0, strxl(sHPath + 0, 3));
	dirG2 = combine_cx(dirG1, strxl(sHPath + 3, 3));
	dirG3 = combine_cx(dirG2, strxl(sHPath + 6, 3));
	dirG4 = combine_cx(dirG3, strxl(sHPath + 9, 3));
	dirHP = combine(dirG4, sHPath);
	dirHI = combine(dirHP, sHInfo);

	if (sHash) // Set
	{
		char *symDir = combine(dirHI, sHash);

		createDirIfNotExist(dirG0);
		createDirIfNotExist(dirG1);
		createDirIfNotExist(dirG2);
		createDirIfNotExist(dirG3);
		createDirIfNotExist(dirG4);
		recurRemoveDirIfExist(dirHP);
		createDir(dirHP);
		createDir(dirHI);
		createDir(symDir);

		memFree(symDir);
	}
	else if (existDir(dirHI)) // Get
	{
		autoList_t *symDirs = lsDirs(dirHI);

		errorCase(getCount(symDirs) != 1);

		sHash = getLine(symDirs, 0);
		eraseParent(sHash);
		toLowerLine(sHash);

		errorCase(!lineExp("<32,09af>", sHash));

		releaseAutoList(symDirs);
	}
//	memFree(dirG0); // dont!
	memFree(dirG1);
	memFree(dirG2);
	memFree(dirG3);
	memFree(dirG4);
	memFree(dirHP);
	memFree(dirHI);
	return sHash;
}
static char *GetOrSetCache(char *sHPath, char *sHInfo, char *sHash)
{
	mutex();
	sHash = GetOrSetCache_NoLock(sHPath, sHInfo, sHash);
	unmutex();
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

	if (!sHash)
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
