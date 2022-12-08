#include "C:\Factory\Common\all.h"

#define VIR_PATH_DLMTR '$'

/*
	パスデリミタは $
	パスの前後の $ は無視する。連続する $ ($$...) は一つの $ と見なす。
	ルートは表現できない。"" や "$" は "_" に置き換えられる。
	ex.
		ABC
		ABC$DEF
		ABC$DEF$GHI
*/
static char *GetFairServerPath(char *svrPath)
{
	autoList_t *sPTkns = tokenize(svrPath, VIR_PATH_DLMTR);
	char *sPTkn;
	uint index;

	trimLines(sPTkns);

	foreach (sPTkns, sPTkn, index)
	{
		setElement(sPTkns, index, (uint)lineToFairLocalPath(sPTkn, 0));
		memFree(sPTkn);
	}
	svrPath = untokenize_cx(sPTkns, xcout("%c", VIR_PATH_DLMTR));

	if (!*svrPath) // ? == ""
	{
		memFree(svrPath);
		svrPath = strx("_"); // 代替
	}
	releaseDim(sPTkns, 1);
	return svrPath;
}
static char *GetFairServerPath_x(char *svrPath)
{
	char *out = GetFairServerPath(svrPath);
	memFree(svrPath);
	return out;
}
static char *GetLocalServerPath(char *svrPath)
{
	autoList_t *sPTkns = tokenize(svrPath, VIR_PATH_DLMTR);
	char *result;

	result = (char *)unaddElement(sPTkns);
	releaseDim(sPTkns, 1);
	return result;
}
static autoList_t *ReadServerPathList(char *file)
{
	autoList_t *lines = readResourceLines(file);
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		setElement(lines, index, (uint)GetFairServerPath_x(line));
	}
	return lines;
}
static void TryCreateParent(char *dir, char *relFile)
{
	autoList_t *rFTkns;
	char *rFTkn;
	uint index;

	cout("TryCreateParent() Start\n");
	cout("dir: %s\n", dir);
	cout("relFile: %s\n", relFile);

	relFile = strx(relFile);
	escapeYen(relFile);
	rFTkns = tokenize(relFile, '/');
	memFree((char *)unaddElement(rFTkns)); // ファイルを作成しない。

	addCwd(dir);

	foreach (rFTkns, rFTkn, index)
	{
		cout("rFTkn: %s\n", rFTkn);

		createDirIfNotExist(rFTkn);
		changeCwd(rFTkn);
	}
	unaddCwd();

	memFree(relFile);
	releaseDim(rFTkns, 1);

	cout("TryCreateParent() End\n");
}

static int AddScriptMode;
static FILE *ScriptFp;

static void OpenScript(char *destFile)
{
	ScriptFp = fileOpen(destFile, AddScriptMode ? "at" : "wt");
}
static void AddToScript(char *command, char *file, char *svrPath, char *newSvrPath)
{
	errorCase(m_isEmpty(command));

	if (m_isEmpty(file))
		file = "(DUMMY)";

	if (m_isEmpty(svrPath))
		svrPath = "(DUMMY)";

	if (m_isEmpty(newSvrPath))
		newSvrPath = "(DUMMY)";

	writeLine(ScriptFp, command);
	writeLine(ScriptFp, file);
	writeLine(ScriptFp, svrPath);
	writeLine(ScriptFp, newSvrPath);
}
static void CloseScript(void)
{
	fileClose(ScriptFp);
}

static void MakeUploadScript(char *path, char *svrPath, int intoSubDirMode, char *destFile)
{
	errorCase(m_isEmpty(path));
	errorCase(m_isEmpty(svrPath));
	errorCase(m_isEmpty(destFile));

	path = makeFullPath(path);
	svrPath = GetFairServerPath(svrPath);
	destFile = makeFullPath(destFile);

	OpenScript(destFile);

	if (existDir(path))
	{
		autoList_t *files = lssFiles(path);
		char *file;
		uint index;

		foreach (files, file, index)
		{
			char *relFile = strx(eraseRoot(file, path));
			char *svrFile;

			replaceChar(relFile, VIR_PATH_DLMTR, 'S');
			replaceChar(relFile, '\\', VIR_PATH_DLMTR);

			svrFile = GetFairServerPath_x(xcout(
				"%s%c%s%c%s"
				,svrPath
				,VIR_PATH_DLMTR
				,intoSubDirMode ? getLocal(path) : "" // path == "x:\\" -> ""
				,VIR_PATH_DLMTR
				,relFile
				));

			AddToScript("UP", file, svrFile, NULL);

			memFree(relFile);
			memFree(svrFile);
		}
		releaseDim(files, 1);
	}
	else
	{
		AddToScript("UP", path, svrPath, NULL); // ファイルとして
	}

	CloseScript();

	memFree(path);
	memFree(svrPath);
	memFree(destFile);
}
static void MakeDownloadScript(char *path, char *svrPath, int intoSubDirMode, int autoCreateParent, char *svrFileListFile, char *destFile)
{
	autoList_t *svrFileList;
	char *svrFile;
	uint index;
	char *lclSvrPath;

	errorCase(m_isEmpty(path));
	errorCase(m_isEmpty(svrPath));
	errorCase(m_isEmpty(svrFileListFile));
	errorCase(m_isEmpty(destFile));

	path = makeFullPath(path);
	svrPath = GetFairServerPath(svrPath);
	svrFileList = ReadServerPathList(svrFileListFile);
	destFile = makeFullPath(destFile);
	lclSvrPath = GetLocalServerPath(svrPath);

	OpenScript(destFile);

	foreach (svrFileList, svrFile, index)
	{
		if (mbsStartsWithICase(svrFile, svrPath))
		{
			int chr = svrFile[strlen(svrPath)];

			if (chr == VIR_PATH_DLMTR)
			{
				char *relFile = strx(svrFile + strlen(svrPath) + 1);
				char *file = strx(path);

				replaceChar(relFile, VIR_PATH_DLMTR, '\\');

				if (intoSubDirMode)
					relFile = combine_cx(lclSvrPath, relFile);

				if (autoCreateParent)
					TryCreateParent(path, relFile);

				file = combine(path, relFile);

				AddToScript("DL", file, svrFile, NULL);

				memFree(relFile);
				memFree(file);
			}
			else if (!chr)
			{
				AddToScript("DL", path, svrPath, NULL); // ファイルとして
			}
		}
	}
	CloseScript();

	memFree(path);
	memFree(svrPath);
	releaseDim(svrFileList, 1);
	memFree(destFile);
	memFree(lclSvrPath);
}
static void MakeMoveScript(char *svrPath, char *newSvrPath, char *svrFileListFile, char *destFile)
{
	autoList_t *svrFileList;
	char *svrFile;
	uint index;

	errorCase(m_isEmpty(svrPath));
	errorCase(m_isEmpty(newSvrPath));
	errorCase(m_isEmpty(svrFileListFile));
	errorCase(m_isEmpty(destFile));

	svrPath = GetFairServerPath(svrPath);
	newSvrPath = GetFairServerPath(newSvrPath);
	svrFileList = ReadServerPathList(svrFileListFile);
	destFile = makeFullPath(destFile);

	OpenScript(destFile);

	foreach (svrFileList, svrFile, index)
	{
		if (mbsStartsWithICase(svrFile, svrPath))
		{
			char *p = svrFile + strlen(svrPath);

			if (*p == VIR_PATH_DLMTR || *p == '\0') // ? ディレクトリ || ファイル
			{
				char *newSvrFile = xcout("%s%s", newSvrPath, p);

				AddToScript("MV", NULL, svrFile, newSvrFile);
				memFree(newSvrFile);
			}
		}
	}
	CloseScript();

	memFree(svrPath);
	memFree(newSvrPath);
	releaseDim(svrFileList, 1);
	memFree(destFile);
}
static void MakeRemoveScript(char *svrPath, int intoSubDirMode, char *svrFileListFile, char *destFile)
{
	autoList_t *svrFileList;
	char *svrFile;
	uint index;

	errorCase(m_isEmpty(svrPath));
	errorCase(m_isEmpty(svrFileListFile));
	errorCase(m_isEmpty(destFile));

	svrPath = GetFairServerPath(svrPath);
	svrFileList = ReadServerPathList(svrFileListFile);
	destFile = makeFullPath(destFile);

	OpenScript(destFile);

	foreach (svrFileList, svrFile, index)
	{
		if (mbsStartsWithICase(svrFile, svrPath))
		{
			int chr = svrFile[strlen(svrPath)];

			if (chr == VIR_PATH_DLMTR || chr == '\0') // ? ディレクトリ || ファイル
			{
				AddToScript("MV", NULL, svrFile, NULL);
			}
		}
	}
	CloseScript();

	memFree(svrPath);
	releaseDim(svrFileList, 1);
	memFree(destFile);
}
static void MakeNormalizeScript(char *svrFileListFile, char *destFile)
{
	autoList_t *svrFileList;
	char *svrFile;
	uint index;

	errorCase(m_isEmpty(svrFileListFile));
	errorCase(m_isEmpty(destFile));

	svrFileList = readResourceLines(svrFileListFile); // 直読み
	destFile = makeFullPath(destFile);

	OpenScript(destFile);

	foreach (svrFileList, svrFile, index)
	{
		char *realSvrFile = lineToFairLocalPath(svrFile, 0); // サーバー上で FiarLocalPath ではあるはず。
		char *trueSvrFile = GetFairServerPath(svrFile);

		if (mbs_stricmp(realSvrFile, trueSvrFile))
		{
			AddToScript("MV", NULL, realSvrFile, trueSvrFile);
		}
		memFree(realSvrFile);
		memFree(trueSvrFile);
	}
	CloseScript();

	releaseDim(svrFileList, 1);
	memFree(destFile);
}

int main(int argc, char **argv)
{
	int intoSubDirMode = 0;
	int autoCraeteParent = 0;

	if (argIs("/S"))
	{
		intoSubDirMode = 1;
	}
	if (argIs("/P"))
	{
		autoCraeteParent = 1;
	}
	if (argIs("/A"))
	{
		AddScriptMode = 1;
	}

	if (argIs("/UP"))
	{
		char *path;
		char *svrPath;
		char *destFile;

		path = nextArg();
		svrPath= nextArg();
		destFile = nextArg();

		MakeUploadScript(path, svrPath, intoSubDirMode, destFile);
		return;
	}
	if (argIs("/DL"))
	{
		char *path;
		char *svrPath;
		char *svrFileListFile;
		char *destFile;

		path = nextArg();
		svrPath= nextArg();
		svrFileListFile = nextArg();
		destFile = nextArg();

		MakeDownloadScript(path, svrPath, intoSubDirMode, autoCraeteParent, svrFileListFile, destFile);
		return;
	}
	if (argIs("/MV"))
	{
		char *svrPath;
		char *newSvrPath;
		char *svrFileListFile;
		char *destFile;

		svrPath= nextArg();
		newSvrPath = nextArg();
		svrFileListFile = nextArg();
		destFile = nextArg();

		MakeMoveScript(svrPath, newSvrPath, svrFileListFile, destFile);
		return;
	}
	if (argIs("/RM"))
	{
		char *svrPath;
		char *destFile;
		char *svrFileListFile;

		svrPath= nextArg();
		svrFileListFile = nextArg();
		destFile = nextArg();

		MakeRemoveScript(svrPath, intoSubDirMode, svrFileListFile, destFile);
		return;
	}
	if (argIs("/Norm"))
	{
		char *svrFileListFile;
		char *destFile;

		svrFileListFile = nextArg();
		destFile = nextArg();

		MakeNormalizeScript(svrFileListFile, destFile);
		return;
	}
}
