/*
	Server.exe [/P ポート番号] [/C 最大同時接続数] [/R ルートDIR] [/D 確保するディスクの空き領域] [/E 停止イベント名] ANTI-MIS-EXEC

		ポート番号     ... 1 ～ 65535, def: 65123
		最大同時接続数 ... 1 ～ IMAX, def: 100
		ルートDIR      ... 過去に指定したことのあるディレクトリ || 空のディレクトリ || createPath(, 'D') 可能なディレクトリ
		確保するディスクの空き領域 ... バイト数を指定する。1 ～ IMAX_64, def: 500 MB
		停止イベント名 ... この名前のイベントにセットすると停止する。
*/

#include "C:\Factory\Common\Options\SockServerTh.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "C:\Factory\Common\Options\CRRandom.h"

#define FILEIO_MAX 20

static uint64 KeepDiskFree = 500000000ui64; // 500 MB
static char *RootDir = "C:\\Factory\\tmp\\FilingCase3\\LongPath_aaaaaaaaaa_bbbbbbbbbb_cccccccccc_dddddddddd_eeeeeeeeee_ffffffffff_gggggggggg";
//static char *RootDir = "C:\\appdata\\FilingCase3\\LongPath_aaaaaaaaaa_bbbbbbbbbb_cccccccccc_dddddddddd_eeeeeeeeee_ffffffffff_gggggggggg"; // 100 文字くらい。// appdata 抑止 @ 2021.3.19
static char *DataDir;
static char *TempDir;
static char *SigFile;
static uint EvStop;
static semaphore_t SmphFileIO;
static critical_t CritCommand;

static int RecvPrmData(SockStream_t *ss, char *dataFile, uint64 dataSize) // ret: ? 成功
{
	uint64 count;

	enterSemaphore(&SmphFileIO);
	{
		FILE *fp = fileOpen(dataFile, "wb");

		for (count = 0; count < dataSize; count++)
		{
			int chr = SockRecvChar(ss);

			if (chr == EOF)
				break;

			writeChar(fp, chr);

			if (count % 30000000ui64 == 0ui64) // 30 MB
			{
				updateDiskSpace_Dir(RootDir);

				if (lastDiskFree < KeepDiskFree)
				{
					cout("実行に必要なディスクの空き領域が不足しているためデータ受信は失敗します。\n");
					break;
				}
			}
		}
		fileClose(fp);
	}
	leaveSemaphore(&SmphFileIO);

	return count == dataSize;
}
static void FC3_SendBlock(SockStream_t *ss, void *block, uint blockSize)
{
	SockSendValue(ss, blockSize);
	SockSendBlock(ss, block, blockSize);
}
static void FC3_SendLine(SockStream_t *ss, char *line)
{
	FC3_SendBlock(ss, line, strlen(line));
}

thread_tls static SockStream_t *SL_SS;

static int SL_Action(struct _finddata_t *i)
{
	char *localPath;

	if (
		!strcmp(i->name, ".") ||
		!strcmp(i->name, "..")
		)
		goto endFunc;

	if (i->attrib & _A_SUBDIR)
		localPath = xcout("%s\\", i->name);
	else
		localPath = strx(i->name);

	FC3_SendLine(SL_SS, localPath);
	memFree(localPath);
endFunc:
	return 1;
}
static void SendResList(SockStream_t *ss, char *relPath)
{
	char *wCard = combine_cx(DataDir, xcout("%s\\*", relPath));

	enterSemaphore(&SmphFileIO);
	{
		SL_SS = ss;
		fileSearch(wCard, SL_Action);
		SL_SS = NULL;
	}
	leaveSemaphore(&SmphFileIO);

	memFree(wCard);

	FC3_SendLine(ss, "/LIST/e");
}
static void SendResFile(SockStream_t *ss, char *relPath)
{
	char *file = combine(DataDir, relPath);

	if (existFile(file))
	{
		enterSemaphore(&SmphFileIO);
		{
			FILE *fp = fileOpen(file, "rb");

			SockSendValue64(ss, getFileSizeFPSS(fp));

			for (; ; )
			{
				autoBlock_t *buff = readBinaryStream(fp, 2000000); // 2 MB

				if (!buff)
					break;

				SockSendBlock(ss, directGetBuffer(buff), getSize(buff));
			}
			fileClose(fp);
		}
		leaveSemaphore(&SmphFileIO);
	}
	else
	{
		SockSendValue64(ss, 0ui64);
	}

	memFree(file);

	FC3_SendLine(ss, "/GET/e");
}
static void PerformTh(int sock, char *strip)
{
	SockStream_t *ss = CreateSockStream2(sock, 0, 30, 0);
	int keepConn;

	LOGPOS();

	do
	{
		char *command;
		char *path;
		char *sDataSize;
		uint64 dataSize;
		char *dataFile;

		keepConn = 0;

		command   = SockRecvLine(ss, 30);
		path      = SockRecvLine(ss, 1000);
		sDataSize = SockRecvLine(ss, 30);

		if (IsEOFSockStream(ss))
		{
			cout("切断されました。\n");
			goto fault;
		}
		coutJLine_x(xcout("command: %s", command));
		coutJLine_x(xcout("path: %s", path));
		coutJLine_x(xcout("dataSize: %s", sDataSize));

		dataSize = toValue64(sDataSize);

		if (!isFairRelPath(path, strlen(DataDir)))
		{
			char *tmp;

			cout("不正なパスです。\n");
			tmp = lineToFairRelPath(path, strlen(DataDir));
			cout("正規化 -> %s\n", tmp);
			memFree(tmp);
			goto fault;
		}
		updateDiskSpace_Dir(RootDir);

		if (lastDiskFree < KeepDiskFree)
		{
			cout("実行に必要なディスクの空き領域が不足しています。\n");
			goto fault;
		}
		if (lastDiskFree - KeepDiskFree < dataSize)
		{
			cout("ディスクの空き領域が要求データサイズに対して不足しています。\n");
			goto fault;
		}
		dataFile = combine_cx(TempDir, MakeUUID(1));

		if (!RecvPrmData(ss, dataFile, dataSize))
		{
			cout("データの受信に失敗しました。\n");
			goto fault2;
		}

		{
			char *ender = SockRecvLine(ss, 30);

			if (_stricmp(ender, "/SEND/e"))
			{
				cout("不正な終端です。\n");
				memFree(ender);
				goto fault2;
			}
			memFree(ender);
		}

		if (!_stricmp(command, "LIST"))
		{
			enterCritical(&CritCommand);
			{
				SendResList(ss, path);
			}
			leaveCritical(&CritCommand);
		}
		else if (!_stricmp(command, "GET"))
		{
			enterCritical(&CritCommand);
			{
				SendResFile(ss, path);
			}
			leaveCritical(&CritCommand);
		}
		else if (!_stricmp(command, "POST"))
		{
			char *file = combine(DataDir, path);

			enterCritical(&CritCommand);
			{
				LOGPOS();
				recurRemovePathIfExist(file);
				LOGPOS();
				createPath(file, 'X');
				LOGPOS();
				moveFile(dataFile, file);
				LOGPOS();
				createFile(dataFile);
				LOGPOS();
			}
			leaveCritical(&CritCommand);

			memFree(file);

			FC3_SendLine(ss, "/POST/e");
		}
		else if (!_stricmp(command, "GET-POST"))
		{
			char *file = combine(DataDir, path);

			enterCritical(&CritCommand);
			{
				SendResFile(ss, path);

				LOGPOS();
				recurRemovePathIfExist(file);
				LOGPOS();
				createPath(file, 'X');
				LOGPOS();
				moveFile(dataFile, file);
				LOGPOS();
				createFile(dataFile);
				LOGPOS();
			}
			leaveCritical(&CritCommand);

			memFree(file);

			FC3_SendLine(ss, "/GET-POST/e");
		}
		else if (!_stricmp(command, "DELETE"))
		{
			char *file = combine(DataDir, path);

			enterCritical(&CritCommand);
			{
				LOGPOS();
				recurRemovePathIfExist(file);
				LOGPOS();
			}
			leaveCritical(&CritCommand);

			memFree(file);

			FC3_SendLine(ss, "/DELETE/e");
		}
		else if (!_stricmp(command, "HELLO"))
		{
			FC3_SendLine(ss, "/HELLO/e");
		}
		else
		{
			cout("不明なコマンドです。\n");
			goto fault2;
		}
		SockFlush(ss);
		keepConn = 1;
		cout("コマンドは正常に実行されました。\n");

	fault2:
		removeFile(dataFile);
		memFree(dataFile);

	fault:
		memFree(command);
		memFree(path);
		memFree(sDataSize);
	}
	while (keepConn);

	LOGPOS();
	ReleaseSockStream(ss);
}
static int IdleTh(void)
{
	static int keep = 1;

	if (EvStop && handleWaitForMillis(EvStop, 0))
		keep = 0;

	while (hasKey())
		if (getKey() == 0x1b)
			keep = 0;

	if (!keep)
		LOGPOS();

	return keep;
}
static int HasOtherRootDirParent(char *dir)
{
	dir = getParent(dir);

	for (; ; )
	{
		char *file = combine(dir, getLocal(SigFile));

		cout("chk_sig: %s\n", file);

		if (existFile(file))
		{
			memFree(file);
			memFree(dir);
			return 1;
		}
		memFree(file);

		if (isAbsRootDir(dir))
			break;

		dir= getParent_x(dir);
	}
	memFree(dir);
	return 0;
}
static void CheckRootDir(void)
{
	if (!existDir(RootDir))
		return;

	if (accessible(SigFile))
		return;

	errorCase_m(lsCount(RootDir), "指定されたディレクトリは、別のアプリケーションによって使用されています。"); // ? 空ではない。

	errorCase_m(HasOtherRootDirParent(RootDir), "指定されたディレクトリは、別のサービスによって使用されています。"); // ? 別の RootDir の配下
}
int main(int argc, char **argv)
{
	uint portNo = 65123;
	uint connectMax = 100;

readArgs:
	if (argIs("/P"))
	{
		portNo = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/C"))
	{
		connectMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/R"))
	{
		RootDir = nextArg();
		goto readArgs;
	}
	if (argIs("/D"))
	{
		KeepDiskFree = toValue64(nextArg());
		goto readArgs;
	}
	if (argIs("/E"))
	{
		char *name = nextArg();

		cout("停止イベント名: %s\n", name);

		errorCase(m_isEmpty(name));

		EvStop = eventOpen(name);
		goto readArgs;
	}

	errorCase_m(strcmp(nextArg(), "ANTI-MIS-EXEC"), "no ANTI-MIS-EXEC");

	cout("ポート番号: %u\n", portNo);
	cout("最大同時接続数: %u\n", connectMax);
	cout("確保するディスクの空き領域: %I64u\n", KeepDiskFree);
	cout("RootDir.0: %s\n", RootDir);

	errorCase(!m_isRange(portNo, 1, 65535));
	errorCase(!m_isRange(connectMax, 1, IMAX));

	RootDir = makeFullPath(RootDir);
	RootDir = toFairFullPathFltr_x(RootDir);

	cout("RootDir.1: %s\n", RootDir);

	DataDir = combine(RootDir, "d");
	TempDir = combine(RootDir, "w");
	SigFile = combine(RootDir, "FilingCase3_{20276b27-459e-4bed-b744-cb8f57c5af91}.sig");

	cout("DataDir: %s\n", DataDir);
	cout("TempDir: %s\n", TempDir);
	cout("SigFile: %s\n", SigFile);

	CheckRootDir();

	createPath(DataDir, 'd');
	createDirIfNotExist(TempDir);
	createFileIfNotExist(SigFile);

	recurClearDir(TempDir);

	initSemaphore(&SmphFileIO, FILEIO_MAX);
	initCritical(&CritCommand);

	sockServerTh(PerformTh, portNo, connectMax, IdleTh);

	memFree(RootDir);
	memFree(DataDir);
	memFree(TempDir);
	memFree(SigFile);

	if (EvStop)
		handleClose(EvStop);

	fnlzSemaphore(&SmphFileIO);
	fnlzCritical(&CritCommand);

	cout("FC3_SERVER_END\n");
}
