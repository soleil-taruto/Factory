#include "all.h"

#define WILDCARD "*"

static FILE *DirsExtraFp;
static FILE *FilesExtraFp;

void (*lsDirAction)(char *dir); // extra-prm
void (*lsFileAction)(char *file); // extra-prm

/*
	例:
		lsInfos = newList();
		paths = ls(DIR);
		infos = lsInfos;
		lsInfos = NULL;

		rapidSortLinesSubColumns(paths, infos, NULL);

	ls のみ併用可能。lss と使うと、順序がぶっ壊れる。
*/
autoList_t *lsInfos; // extra-prm

static void AddPath(autoList_t *paths, char *path, FILE *extra_fp, void (*lsAction)(char *), autoList_t *infos)
{
	if (lsAction)
		lsAction(path);

	if (extra_fp)
	{
		writeLine(extra_fp, path);
		memFree(path);
	}
	else
	{
		addElement(paths, (uint)path);
	}

	if (infos)
	{
		lsInfo_t *i = (lsInfo_t *)memAlloc(sizeof(lsInfo_t));

		i->attrArch     = m_01(lastFindData.attrib & _A_ARCH);
		i->attrHidden   = m_01(lastFindData.attrib & _A_HIDDEN);
		i->attrReadOnly = m_01(lastFindData.attrib & _A_RDONLY);
		i->attrSubDir   = m_01(lastFindData.attrib & _A_SUBDIR);
		i->attrSystem   = m_01(lastFindData.attrib & _A_SYSTEM);
		i->createTime   = lastFindData.time_create;
		i->accessTime   = lastFindData.time_access;
		i->writeTime    = lastFindData.time_write;

		addElement(infos, (uint)i);
	}
}

int antiSubversion; // extra-prm
int ignoreUtfPath; // extra-prm
uint findLimiter; // extra-prm
int (*findAcceptName)(char *name); // extra-prm

/*
	unsigned attrib;
		_A_ARCH
		_A_HIDDEN
		_A_NORMAL
		_A_RDONLY
		_A_SUBDIR
		_A_SYSTEM

	time_t time_create;
	time_t time_access;
	time_t time_write;
	_fsize_t size;
	char name[_MAX_PATH];
*/
struct _finddata_t lastFindData;

uint lastDirCount;

autoList_t *ls(char *dir)
{
	autoList_t *paths = createAutoList(128);
	autoList_t *files = createAutoList(128);
	autoList_t *fileInfos = lsInfos ? createAutoList(128) : NULL;
	intptr_t h;
	char *absDir = makeFullPath(dir);
	char *wCard;
	char *name;
	char *path;

	errorCase(!existDir(absDir));

	wCard = combine(absDir, WILDCARD);
	h = _findfirst(wCard, &lastFindData);
	memFree(wCard);

	if (h != -1)
	{
		do
		{
			name = lastFindData.name;

			if (name[0] == '.' && (name[1] == '\0' || name[1] == '.' && name[2] == '\0')) // ".", ".." を除外
			{
				continue;
			}
			if (antiSubversion)
			{
				// .svn は SVN_ASP_DOT_NET_HACK=1 で _svn になるらしい。
				if ((!_stricmp(name, ".svn") || !_stricmp(name, "_svn")) && lastFindData.attrib & _A_SUBDIR)
				{
					continue;
				}
				if (!_stricmp(getExt(name), "rum") && lastFindData.attrib & _A_SUBDIR)
				{
					continue;
				}
			}
			if (strchr(name, '?')) // ? 変な文字を含む
			{
				cout("<UTF-PATH>\n");
				cout("%s\n", absDir);
				cout("%s\n", name);

				if (ignoreUtfPath)
					continue;

				error();
			}
			if (findLimiter)
			{
				if (findLimiter == 1)
					continue;

				findLimiter--;
			}
			if (findAcceptName && !findAcceptName(name))
			{
				continue;
			}
			path = combine(absDir, name);

			if (lastFindData.attrib & _A_SUBDIR) // ? dir
			{
				AddPath(paths, path, DirsExtraFp, lsDirAction, lsInfos);
			}
			else // ? file
			{
				AddPath(files, path, FilesExtraFp, lsFileAction, fileInfos);
			}
		}
		while (_findnext(h, &lastFindData) == 0);

		_findclose(h);
	}
	lastDirCount = getCount(paths);

	addElements(paths, files);
	fixElements(paths);

	if (lsInfos)
	{
		addElements(lsInfos, fileInfos);
		fixElements(lsInfos);

		releaseAutoList(fileInfos);
	}
	releaseAutoList(files);
	memFree(absDir);
	return paths;
}
autoList_t *lss(char *dir)
{
	autoList_t *paths = ls(dir);
	autoList_t *files = createAutoList(128);
	autoList_t tmplist;
	uint dircnt;
	uint index;

	addElements(files, gndFollowElementsVar(paths, lastDirCount, tmplist));
	setCount(paths, lastDirCount);

	dircnt = lastDirCount;

	for (index = 0; index < dircnt; index++)
	{
		char *dir2 = getLine(paths, index);
		autoList_t *subPaths;
		autoList_t subDirs;
		autoList_t subFiles;

		subPaths = lss(dir2);
		subDirs = gndSubElements(subPaths, 0, lastDirCount);
		subFiles = gndFollowElements(subPaths, lastDirCount);

		addElements(paths, &subDirs);
		addElements(files, &subFiles);

		releaseAutoList(subPaths);
	}
	lastDirCount = getCount(paths);

	addElements(paths, files);
	fixElements(paths);

	releaseAutoList(files);
	return paths;
}

/*
	path == ルートディレクトリのとき _findfirst() は -1 を返す。-> 本関数はディレクトリとして扱う。
*/
void updateFindData(char *path)
{
	intptr_t h = _findfirst(path, &lastFindData);

	if (h == -1)
	{
		if (isRootDir(path))
		{
			memset(&lastFindData, 0x00, sizeof(struct _finddata_t));

			lastFindData.attrib = _A_SUBDIR;
			lastFindData.name[0] = '\\'; // ルートディレクトリであるかどうかの判定に使ってね。

			return;
		}
		error();
	}
	_findclose(h);
}
/*
	ルートディレクトリ無視
	ネットワークパスはこちらを使ってね。
*/
int tryUpdateFindData(char *path)
{
	intptr_t h = _findfirst(path, &lastFindData);

	if (h == -1)
		return 0;

	_findclose(h);
	return 1;
}

time_t getFileAccessTime(char *file)
{
	updateFindData(file);
	return lastFindData.time_access;
}
time_t getFileCreateTime(char *file)
{
	updateFindData(file);
	return lastFindData.time_create;
}
time_t getFileModifyTime(char *file)
{
	updateFindData(file);
	return m_max(lastFindData.time_create, lastFindData.time_write);
}
time_t getFileWriteTime(char *file)
{
	updateFindData(file);
	return lastFindData.time_write;
}

static autoList_t *GetPaths(char *dir, int intoSubDir, int filterDir)
{
	autoList_t *paths = ( !intoSubDir ? ls : lss )(dir);

	if (!filterDir)
		rmtrimSubLines(paths, 0, lastDirCount);
	else
		rmtrimFollowLines(paths, lastDirCount);

	return paths;
}
autoList_t *lsFiles(char *dir)
{
	return GetPaths(dir, 0, 0);
}
autoList_t *lsDirs(char *dir)
{
	return GetPaths(dir, 0, 1);
}
autoList_t *lssFiles(char *dir)
{
	return GetPaths(dir, 1, 0);
}
autoList_t *lssDirs(char *dir)
{
	return GetPaths(dir, 1, 1);
}

static autoList_t *GetSortedPaths(char *dir, int intoSubDir, int filterDir)
{
	autoList_t *paths = GetPaths(dir, intoSubDir, filterDir);

	sortJLinesICase(paths);
	return paths;
}
autoList_t *slsFiles(char *dir)
{
	return GetSortedPaths(dir, 0, 0);
}
autoList_t *slsDirs(char *dir)
{
	return GetSortedPaths(dir, 0, 1);
}
autoList_t *slssFiles(char *dir)
{
	return GetSortedPaths(dir, 1, 0);
}
autoList_t *slssDirs(char *dir)
{
	return GetSortedPaths(dir, 1, 1);
}

void ls2File(char *dir, char *dirsFile, char *filesFile)
{
	DirsExtraFp = fileOpen(dirsFile, "wt");
	FilesExtraFp = fileOpen(filesFile, "wt");

	releaseDim(ls(dir), 1);

	fileClose(DirsExtraFp);
	fileClose(FilesExtraFp);
	DirsExtraFp = NULL;
	FilesExtraFp = NULL;
}
void lss2File(char *dir, char *dirsFile, char *filesFile)
{
	char *entryDirsFile = makeTempFile("entry-dirs");
	char *underDirsFile = makeTempFile("under-dirs");

	createFile(dirsFile);

	DirsExtraFp = fileOpen(entryDirsFile, "wt");
	FilesExtraFp = fileOpen(filesFile, "wt");

	releaseDim(ls(dir), 1);

	fileClose(DirsExtraFp);

	while (getFileSize(entryDirsFile) != 0ui64)
	{
		FILE *fp = fileOpen(entryDirsFile, "rt");
		char *dir;
		char *p;

		DirsExtraFp = fileOpen(underDirsFile, "wt");

		while (dir = readLine(fp))
		{
			releaseDim(ls(dir), 1);
			memFree(dir);
		}
		fileClose(fp);
		fileClose(DirsExtraFp);

		joinFile(dirsFile, entryDirsFile);

		p = entryDirsFile;
		entryDirsFile = underDirsFile;
		underDirsFile = p;
	}
	fileClose(FilesExtraFp);
	DirsExtraFp = NULL;
	FilesExtraFp = NULL;

	removeFile(entryDirsFile);
	removeFile(underDirsFile);
	memFree(entryDirsFile);
	memFree(underDirsFile);
}

void fileSearch(char *wCard, int (*action)(struct _finddata_t *))
{
	intptr_t h;
	struct _finddata_t findData;

	h = _findfirst(wCard, &findData);

	if (h == -1)
		return;

	do
	{
		if (!action(&findData))
			break;
	}
	while (_findnext(h, &findData) == 0);

	_findclose(h);
}

static uint FSC_Count;

static int FSC_Action(struct _finddata_t *i)
{
	FSC_Count++;
	return 1;
}
uint fileSearchCount(char *wCard)
{
	FSC_Count = 0;
	fileSearch(wCard, FSC_Action);
	return FSC_Count;
}

static int FSX_Found;

static int FSX_Action(struct _finddata_t *i)
{
	FSX_Found = 1;
	return 0;
}
int fileSearchExist(char *wCard)
{
	FSX_Found = 0;
	fileSearch(wCard, FSX_Action);
	return FSX_Found;
}

uint lsCount(char *dir)
{
	uint count = 0;

	dir = makeFullPath(dir);

	if (existDir(dir))
	{
		char *wCard = combine(dir, WILDCARD);
		intptr_t h;
		struct _finddata_t findData;

		h = _findfirst(wCard, &findData);

		if (h != -1)
		{
			do
			{
				// 除外
				{
					const char *name = findData.name;

					if (name[0] == '.' && (name[1] == '\0' || name[1] == '.' && name[2] == '\0')) // ".", ".." を除外
						continue;
				}

				count++;
			}
			while (_findnext(h, &findData) == 0);

			_findclose(h);
		}
		memFree(wCard);
	}
	memFree(dir);

	return count;
}

// ---- cmdDir ----

static void CD_ExecBatch(char *dir, char *trailOpts, char *outFile)
{
	char *batFile = makeTempPath("bat");
	char *midFile = makeTempPath(NULL);
	char *erroutFile = makeTempPath(NULL);
	FILE *rfp;
	FILE *wfp;

	wfp = fileOpen(batFile, "wt");
	writeLine(wfp, "@ECHO OFF");
	writeLine(wfp, "SET DIRCMD=");
	writeLine_x(wfp, xcout("DIR \"%s\" %s 1> \"%s\" 2> \"%s\"", dir, trailOpts, midFile, erroutFile));
	fileClose(wfp);

	execute_x(xcout("CMD /C \"%s\"", batFile));

//	dir = makeFullPath(dir); // ネットワークパス対応 @ 2017.11.2

	rfp = fileOpen(midFile, "rt");
	wfp = fileOpen(outFile, "at");

	/*
		ファイル・ディレクトリが１つも無ければ、midFile は空(0バイト)になる。
		0 バイトのファイルを開くと、readLine() は初回から NULL を返す。-> ""(空行)が追加されることはない！
	*/

	for (; ; )
	{
		char *line = readLine(rfp);
		char *path;

		if (!line)
			break;

		if (!*line) // これは無いはずだけど、念のため。
		{
			LOGPOS();
			memFree(line);
			continue;
		}

//		path = combine(dir, line);
		path = xcout("%s\\%s", dir, line); // ネットワークパス対応 @ 2017.11.2

		writeLine(wfp, path);
		memFree(line);
		memFree(path);
	}
	fileClose(rfp);
	fileClose(wfp);

	removeFile_x(batFile);
	removeFile_x(midFile);
	removeFile_x(erroutFile);

//	memFree(dir); // ネットワークパス対応 @ 2017.11.2
}
void cmdDir_ls2File_noClear(char *dir, char *dirsFile, char *filesFile)
{
	CD_ExecBatch(dir, "/AD /B", dirsFile);
	CD_ExecBatch(dir, "/A-D /B", filesFile);
}
void cmdDir_ls2File(char *dir, char *dirsFile, char *filesFile)
{
	removeFileIfExist(dirsFile);
	removeFileIfExist(filesFile);

	cmdDir_ls2File_noClear(dir, dirsFile, filesFile);
}
void cmdDir_lss2File(char *dir, char *dirsFile, char *filesFile)
{
	FILE *fp;

	cmdDir_ls2File(dir, dirsFile, filesFile);

	fp = fileOpen(dirsFile, "rt");

	for (; ; )
	{
		char *subDir = readLine(fp);

		if (!subDir)
			break;

		cmdDir_ls2File_noClear(subDir, dirsFile, filesFile);
	}
	fileClose(fp);
}
static autoList_t *CD_CallFunc(char *dir, void (*func)(char *, char *, char *), int dirMode)
{
	char *dirsFile = makeTempPath(NULL);
	char *filesFile = makeTempPath(NULL);
	autoList_t *ret;

	func(dir, dirsFile, filesFile);

	/*
		ファイル・ディレクトリが１つも無ければ、dirsFile, filesFile は空(0バイト)になる。
		0 バイトのファイルを readLines() すると { } を返す。-> { "" } とか、空行を含むリストを返すことはない！
	*/

	if (dirMode)
		ret = readLines(dirsFile);
	else
		ret = readLines(filesFile);

	removeFile_x(dirsFile);
	removeFile_x(filesFile);
	return ret;
}
autoList_t *cmdDir_lsFiles(char *dir)
{
	return CD_CallFunc(dir, cmdDir_ls2File, 0);
}
autoList_t *cmdDir_lsDirs(char *dir)
{
	return CD_CallFunc(dir, cmdDir_ls2File, 1);
}
autoList_t *cmdDir_lssFiles(char *dir)
{
	return CD_CallFunc(dir, cmdDir_lss2File, 0);
}
autoList_t *cmdDir_lssDirs(char *dir)
{
	return CD_CallFunc(dir, cmdDir_lss2File, 1);
}

// ----
