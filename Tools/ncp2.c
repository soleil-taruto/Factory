/*
	ncp2.exe [/S SERVER-DOMAIN] [/P SERVER-PORT] [/R RETRY-COUNT] [/T RETRY-WAIT-MILLIS] ...

	ncp2.exe ... (/MUP | MUP) LOCAL-DIR [SERVER-DIR]

	ncp2.exe ... (/MDL | MDL) LOCAL-DIR SERVER-DIR

	ncp2.exe ... (/MDL | MDL) * SERVER-DIR

	- - -

	/MUP, /MDL 共に、コピー先に既に存在するファイルについては（内容が異なっていても）更新しないことに注意して下さい。

	/MUP, /MDL 共に、コピー先にしか存在しないファイル・フォルダを削除することに注意して下さい。

	エスケープキーを押すと安全に中断します。
	同じコマンドを実行することによって中断したところから再開できます。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

static char *ServerDomain;
static uint ServerPort = UINTMAX;
static uint RetryCount = UINTMAX;
static uint RetryWaitMillis = UINTMAX;

#define NCP_EXE "C:\\Factory\\Tools\\ncp.exe"

#define MIRROR_DIR_RETRY_COUNT 2
#define MIRROR_DIR_RETRY_WAIT_MILLIS 3000

static int AbortRequested;

static void CheckAbortRequest(void)
{
	int escapePressed = 0;

	while (hasKey())
		if (getKey() == 0x1b)
			escapePressed = 1;

	if (!AbortRequested && escapePressed)
	{
		AbortRequested = 1;

		cout("+------------+\n");
		cout("| 中断します |\n");
		cout("+------------+\n");
	}
}
static char *GetNcpOptions(void) // ret: c_
{
	static char *ret;

	CheckAbortRequest();

	if (!ret)
		ret = strx("");

	*ret = '\0';

	if (ServerDomain)
		ret = addLine_x(ret, xcout(" /S \"%s\"", ServerDomain));

	if (ServerPort != UINTMAX)
		ret = addLine_x(ret, xcout(" /P %u", ServerPort));

	if (RetryCount != UINTMAX)
		ret = addLine_x(ret, xcout(" /R %u", RetryCount));

	if (RetryWaitMillis != UINTMAX)
		ret = addLine_x(ret, xcout(" /T %u", RetryWaitMillis));

	return ret;
}
static autoList_t *GetServerPaths(char *serverDir)
{
	char *molp = MakeUUID(1);
	char *outFile = makeTempPath(NULL);
	autoList_t *serverPaths = NULL;

	coExecute_x(xcout(NCP_EXE " //MOLP %s //O %s%s /LS \"%s\"", molp, outFile, GetNcpOptions(), serverDir));

	if (!lastSystemRet) // ? コマンド実行成功
	{
		char *line;
		uint index;

		serverPaths = readLines(outFile);

		foreach (serverPaths, line, index)
		{
			if (startsWith(line, molp))
				eraseLine(line, strlen(molp));
			else
				*line = '\0';
		}
		trimLines(serverPaths);
	}
	memFree(molp);
	removeFile(outFile);
	memFree(outFile);
	return serverPaths;
}
static int RemoveServerPath(char *serverPath)
{
	coExecute_x(xcout(NCP_EXE "%s /RM \"%s\"", GetNcpOptions(), serverPath));

	return !lastSystemRet && !AbortRequested;
}
static int UploadFile(char *clientFile, char *serverFile)
{
	coExecute_x(xcout(NCP_EXE "%s /UP \"%s\" \"%s\"", GetNcpOptions(), clientFile, serverFile));

	return !lastSystemRet && !AbortRequested;
}
static int DownloadFile(char *clientFile, char *serverFile)
{
	coExecute_x(xcout(NCP_EXE "%s /DL \"%s\" \"%s\"", GetNcpOptions(), clientFile, serverFile));

	return !lastSystemRet && !AbortRequested;
}
static int MirrorDirMain(char *clientDir, char *serverDir, int direction)
{
	autoList_t *serverPaths = GetServerPaths(serverDir);
	autoList_t *serverDirs;
	autoList_t *serverFiles;
	int retval = 0;

	if (!serverPaths)
		goto endFunc;

	retval = 1;

	serverDirs  = newList();
	serverFiles = newList();

	{
		char *serverPath;
		uint index;

		foreach (serverPaths, serverPath, index)
		{
			int dirFlag = 0;

			escapeYen(serverPath);

			if (endsWith(serverPath, "/"))
			{
				strchr(serverPath, '\0')[-1] = '\0';
				dirFlag = 1;
			}
			restoreYen(serverPath);

			if (dirFlag)
				addElement(serverDirs, (uint)getLocal(serverPath));
			else
				addElement(serverFiles, (uint)getLocal(serverPath));
		}
	}

	if (direction == 'U') // Upload
	{
		char *dummyFile = makeTempFile(NULL);
		char *dummyServerPath = combine_cx(serverDir, addExt(MakeUUID(1), "ncp2_dummy.tmp"));

		// 空フォルダ対策
		retval =
			UploadFile(dummyFile, dummyServerPath) &&
			RemoveServerPath(dummyServerPath);

		removeFile(dummyFile);
		memFree(dummyFile);
		memFree(dummyServerPath);
	}
	else // Download
	{
		createDirIfNotExist(clientDir);
	}

	if (!retval)
		goto freeVars;

	{
		autoList_t *dirs = lsDirs(clientDir);
		char *dir;
		uint index;
		autoList_t *bothExistDirs;

		eraseParents(dirs);

		bothExistDirs = mergeConstLinesICase(dirs, serverDirs);

		foreach (bothExistDirs, dir, index)
		{
			char *clientSubDir = combine(clientDir, dir);
			char *serverSubDir = combine(serverDir, dir);

			retval = MirrorDirMain(clientSubDir, serverSubDir, direction);

			memFree(clientSubDir);
			memFree(serverSubDir);

			if (!retval)
				break;
		}
		releaseDim(bothExistDirs, 1);

		if (!retval)
			goto endClientOnlyDirs;

		foreach (dirs, dir, index)
		{
			char *clientSubDir = combine(clientDir, dir);
			char *serverSubDir = combine(serverDir, dir);

			if (direction == 'U') // Upload
			{
				retval = MirrorDirMain(clientSubDir, serverSubDir, direction);
			}
			else // Download
			{
				recurRemoveDir(clientSubDir);
			}
			memFree(clientSubDir);
			memFree(serverSubDir);

			if (!retval)
				break;
		}
	endClientOnlyDirs:
		releaseDim(dirs, 1);

		if (!retval)
			goto freeVars;

		foreach (serverDirs, dir, index)
		{
			char *clientSubDir = combine(clientDir, dir);
			char *serverSubDir = combine(serverDir, dir);

			if (direction == 'U') // Upload
			{
				retval = RemoveServerPath(serverSubDir);
			}
			else // Download
			{
				retval = MirrorDirMain(clientSubDir, serverSubDir, direction);
			}
			memFree(clientSubDir);
			memFree(serverSubDir);

			if (!retval)
				break;
		}
	}

	if (!retval)
		goto freeVars;

	{
		autoList_t *files = lsFiles(clientDir);
		char *file;
		uint index;

		eraseParents(files);

		releaseDim(mergeConstLinesICase(files, serverFiles), 1);

		foreach (files, file, index)
		{
			char *clientFile = combine(clientDir, file);
			char *serverFile = combine(serverDir, file);

			if (direction == 'U') // Upload
			{
				retval = UploadFile(clientFile, serverFile);
			}
			else // Download
			{
				removeFile(clientFile);
			}
			memFree(clientFile);
			memFree(serverFile);

			if (!retval)
				break;
		}
		releaseDim(files, 1);

		if (!retval)
			goto freeVars;

		foreach (serverFiles, file, index)
		{
			char *clientFile = combine(clientDir, file);
			char *serverFile = combine(serverDir, file);

			if (direction == 'U') // Upload
			{
				retval = RemoveServerPath(serverFile);
			}
			else // Download
			{
				retval = DownloadFile(clientFile, serverFile);
			}
			memFree(clientFile);
			memFree(serverFile);

			if (!retval)
				break;
		}
	}

freeVars:
	releaseDim(serverPaths, 1);
	releaseAutoList(serverDirs);
	releaseAutoList(serverFiles);

endFunc:
	cout("retval: %d\n", retval);
	return retval;
}
static void MirrorDir(char *clientDir, char *serverDir, int direction)
{
	uint retryCount = 0;

	while (!MirrorDirMain(clientDir, serverDir, direction) && !AbortRequested)
	{
		if (MIRROR_DIR_RETRY_COUNT < ++retryCount)
		{
			cout("+--------------------------------------------------------+\n");
			cout("| 失敗しましたがリトライ回数の上限に達したので終了します |\n");
			cout("+--------------------------------------------------------+\n");

			break;
		}
		cout("リトライ %u 回目\n", retryCount);
		coSleep(MIRROR_DIR_RETRY_WAIT_MILLIS);
	}

	if (AbortRequested)
	{
		cout("中断しました。\n");
	}
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/S"))
	{
		ServerDomain = nextArg();
		goto readArgs;
	}
	if (argIs("/P"))
	{
		ServerPort = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/R"))
	{
		RetryCount = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/RWTM"))
	{
		RetryWaitMillis = toValue(nextArg());
		goto readArgs;
	}

	if (argIs("/MUP") || argIs("MUP")) // Mirror Upload
	{
		char *clientDir;
		char *serverDir;

		cout("MIRROR UPLOAD\n");

		clientDir = nextArg();

		if (clientDir[0] == '*')
			clientDir = dropDirFile(); // g

		if (hasArgs(1))
			serverDir = nextArg();
		else
			serverDir = getLocal(clientDir);

		cout("< %s\n", clientDir);
		cout("> %s\n", serverDir);

		errorCase(!*clientDir);
		errorCase(!*serverDir);

		errorCase(!existDir(clientDir));
		errorCase(!*serverDir);

		MirrorDir(clientDir, serverDir, 'U');
	}
	else if (argIs("/MDL") || argIs("MDL")) // Mirror Download
	{
		char *clientDir;
		char *serverDir;
		char *willOpenDir = NULL;
		int type;

		cout("MIRROR DOWNLOAD\n");

		clientDir = nextArg();
		serverDir = nextArg();

		errorCase(!*clientDir);
		errorCase(!*serverDir);

		if (clientDir[0] == '*')
			clientDir = combine(willOpenDir = makeFreeDir(), getLocal(serverDir)); // g

		cout("> %s\n", clientDir);
		cout("< %s\n", serverDir);

		errorCase(!existDir(clientDir) && !creatable(clientDir));

		MirrorDir(clientDir, serverDir, 'D');
	}
	else
	{
		cout("不明なコマンド\n");
	}
}
