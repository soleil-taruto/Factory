#include "C:\Factory\Common\all.h"

#define REV_MIN 3
#define REV_MAX 100
#define REV_TOTAL_SIZE_MAX 1000000000ui64 // 1 GB

#define VER_MIN 3
#define VER_MAX 100
#define VER_TOTAL_SIZE_MAX 1000000000ui64 // 1 GB

#define BETA_MIN 3
#define BETA_MAX 10
#define BETA_TOTAL_SIZE_MAX 150000000ui64 // 150 MB

#define GAME_ORDER_FILE "order.txt"
#define GAME_TITLE_FILE "title.txt"

static int IsAsciiStr(char *str)
{
	char *p;

	for (p = str; *p; p++)
		if (!m_isRange(*p, '\x21', '\x7e'))
			return 0;

	return 1;
}
static char *GetRevision(void)
{
	char *file = makeTempFile(NULL);
	char *revision;

	coExecute_x(xcout("C:\\Factory\\DevTools\\rev.exe /P > \"%s\"", file));
	revision = readFirstLine(file);

	errorCase(!lineExp("<4,09>.<3,09>.<5,09>", revision)); // 2bs

	removeFile(file);
	memFree(file);
	return revision;
}
static uint64 GetTotalSize_Paths(autoList_t *paths)
{
	uint64 totalSize = 0;
	char *path;
	uint index;

	LOGPOS();
	paths = copyLines(paths);

	foreach (paths, path, index)
	{
		if (existDir(path))
			addElements_x(paths, ls(path));
		else
			totalSize += getFileSize(path);
	}
	releaseDim(paths, 1);
	cout("totalSize: %I64u\n", totalSize);
	return totalSize;
}
static void TrimRev(char *appDir)
{
	autoList_t *revDirs = lsDirs(appDir);

	sortJLinesICase(revDirs);
	reverseElements(revDirs); // 終端 == 最も旧いリビジョン

	while (REV_MIN < getCount(revDirs) && (REV_MAX < getCount(revDirs) || REV_TOTAL_SIZE_MAX < GetTotalSize_Paths(revDirs)))
	{
		char *revDir = (char *)unaddElement(revDirs); // 最も旧いリビジョンを取り出す。

		cout("[DEL_REV] %s\n", revDir);

		errorCase(!lineExp("<4,09>.<3,09>.<5,09>", getLocal(revDir))); // 2bs

		recurRemoveDir(revDir);
		memFree(revDir);
	}
	releaseDim(revDirs, 1);
}
static void TrimVer(char *appDir)
{
	autoList_t *files = lsFiles(appDir);
	char *file;
	uint index;

	LOGPOS();

	foreach (files, file, index)
		if (!lineExpICase("<>_v<3,09>.zip", file))
			*file = '\0';

	trimLines(files);
	sortJLinesICase(files);
	reverseElements(files); // 終端 == 最も旧いリビジョン

	foreach (files, file, index)
		cout("[VER] %s\n", file); // test-out

	while (VER_MIN < getCount(files) && (VER_MAX < getCount(files) || VER_TOTAL_SIZE_MAX < GetTotalSize_Paths(files)))
	{
		file = (char *)unaddElement(files);

		cout("[DEL_VER.1] %s\n", file);

		removeFile(file);
		file = addExt(file, "md5");

		cout("[DEL_VER.2] %s\n", file);

		removeFileIfExist(file); // 存在するはずだけど、念の為 *IfExist にする。
		memFree(file);
	}
	releaseDim(files, 1);

	LOGPOS();
}
static void TrimBeta(char *appDir)
{
	autoList_t *files = lsFiles(appDir);
	char *file;
	uint index;

	LOGPOS();

	foreach (files, file, index)
		if (!lineExpICase("<>_BETA_<14,09>.zip", file))
			*file = '\0';

	trimLines(files);
	sortJLinesICase(files);
	reverseElements(files); // 終端 == 最も旧いリビジョン

	foreach (files, file, index)
		cout("[BETA] %s\n", file); // test-out

	while (BETA_MIN < getCount(files) && (BETA_MAX < getCount(files) || BETA_TOTAL_SIZE_MAX < GetTotalSize_Paths(files)))
	{
		file = (char *)unaddElement(files);

		cout("[DEL_BETA.1] %s\n", file);

		removeFile(file);
		file = addExt(file, "md5");

		cout("[DEL_BETA.2] %s\n", file);

		removeFileIfExist(file); // 存在するはずだけど、念の為 *IfExist にする。
		memFree(file);
	}
	releaseDim(files, 1);

	LOGPOS();
}
static void AddRev_File(char *arcFile, char *docRoot)
{
	char *localFile = getLocal(arcFile);
	char *ext;
	char *appName = NULL;
	char *appDir = NULL;
	char *revision = NULL;
	char *revDir = NULL;
	char *wFile = NULL;

	cout("arcFile: %s\n", arcFile);
	cout("docRoot: %s\n", docRoot);

	ext = getExt(localFile);
	appName= changeExt(localFile, "");

	if (!IsAsciiStr(appName))
	{
		cout("アスキーコードの文字列じゃないのでスキップ\n");
		goto endFunc;
	}
	appDir = combine(docRoot, appName);
	cout("appDir: %s\n", appDir);

	createDirIfNotExist(appDir);

	revision = GetRevision();
	revDir = combine(appDir, revision);
	cout("revDir: %s\n", revDir);

	createDir(revDir);

	wFile = combine(revDir, localFile);
	cout("wFile: %s\n", wFile);

	moveFile(arcFile, wFile);

	cout("★★★移動しました★★★\n");

	TrimRev(appDir);

endFunc:
	memFree(appName);
	memFree(appDir);
	memFree(revision);
	memFree(revDir);
	memFree(wFile);
}
static void AddGameVer(char *arcFile, char *rootDir, int beta)
{
	char *arcLocalFile;
	char *name;
	char *wDir;
	char *wFile;
	char *md5File;
	char *wMD5File;

	LOGPOS();

	arcLocalFile = getLocal(arcFile);
	name = strxl(arcLocalFile, strlen(arcLocalFile) - (beta ? 24 : 9)); // "_BETA_11110222033333.zip" or "_v999.zip" を削る。
	wDir = combine(rootDir, name);
	wFile = combine(wDir, arcLocalFile);
	md5File = addExt(strx(arcFile), "md5");
	wMD5File = addExt(strx(wFile), "md5");

	cout("< %s\n", arcFile);
	cout("< %s\n", md5File);
	cout("> %s\n", rootDir);
	cout("1.> %s\n", arcLocalFile);
	cout("2.> %s\n", name);
	cout("3.> %s\n", wDir);
	cout("4.> %s\n", wFile);
	cout("5.> %s\n", wMD5File);

	if (!existFile(md5File)) // ? .md5 ファイルが無い。
		goto cancel;

	if (existFile(wFile) || existFile(wMD5File)) // ? このバージョンは既に存在する。
		goto cancel;

	if (!existDir(wDir))
	{
		autoList_t *lines;

		addCwd(rootDir);
		lines = readResourceLines(GAME_ORDER_FILE);
		insertElement(lines, 0, (uint)strx(name)); // 仮追加
		writeLines_cx(GAME_ORDER_FILE, lines);
		unaddCwd();

		createDir(wDir);

		addCwd(wDir);
		writeOneLine(GAME_TITLE_FILE, name); // 仮作成
		unaddCwd();
	}
	moveFile(arcFile, wFile);
	moveFile(md5File, wMD5File);

	LOGPOS();
	TrimVer(wDir);
	LOGPOS();
	TrimBeta(wDir);
	LOGPOS();

cancel:
	memFree(name);
	memFree(wDir);
	memFree(wFile);
	memFree(md5File);
	memFree(wMD5File);

	LOGPOS();
}
static void ExtractCluster(char *cluster, char *rootDir)
{
	char *lCluster;
	char *node;
	char *wDir;

	LOGPOS();

	lCluster = getLocal(cluster);
	node = changeExt(lCluster, "");
	wDir = combine(rootDir, node);

	cout("wDir: %s\n", wDir);

	if (existDir(wDir))
		coExecute_x(xcout("C:\\Factory\\Tools\\Cluster.exe /OAD /OW /R \"%s\" \"%s\"", cluster, wDir));

	memFree(node);
	memFree(wDir);

	LOGPOS();
}
static void AddRev(char *rDir, char *wDir, char *gameWDir, char *extCluWDir)
{
	rDir = makeFullPath(rDir);
	wDir = makeFullPath(wDir);
	extCluWDir = makeFullPath(extCluWDir);

	cout("< %s\n", rDir);
	cout("> %s\n", wDir);
	cout("> %s\n", gameWDir);
	cout("> %s\n", extCluWDir);

	errorCase(!existDir(rDir));
	errorCase(!existDir(wDir));
	errorCase(!existDir(gameWDir));
	errorCase(!existDir(extCluWDir));

	{
		autoList_t *files = lsFiles(rDir);
		char *file;
		uint index;

		foreach (files, file, index)
		{
			if (!existFile(file)) // .md5 ファイルなど、消失する場合がある。
				continue;

			if (!_stricmp("clu", getExt(file)))
			{
				ExtractCluster(file, extCluWDir);
			}
			else if (lineExpICase("<>_v<3,09>.zip", file))
			{
				AddGameVer(file, gameWDir, 0);
			}
			else if (lineExpICase("<>_BETA_<14,09>.zip", file))
			{
				AddGameVer(file, gameWDir, 1);
			}
			else if (!_stricmp("zip", getExt(file)))
			{
				AddRev_File(file, wDir);
			}
		}
		releaseDim(files, 1);
	}

	memFree(rDir);
	memFree(wDir);
	memFree(extCluWDir);
}
int main(int argc, char **argv)
{
	if (hasArgs(3))
	{
		AddRev(getArg(0), getArg(1), getArg(2), getArg(3));
		return;
	}
	AddRev(
		"C:\\pub\\リリース物",
		"C:\\BlueFish\\BlueFish\\HTT\\stackprobe\\home",
		"C:\\BlueFish\\BlueFish\\HTT\\cerulean\\home\\charlotte",
		"C:\\BlueFish\\BlueFish\\HTT\\extra"
		);
}
