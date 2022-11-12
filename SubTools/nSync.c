/*
	nSync.exe [/T time-margin-sec] [/TC | /TCW] [/HPW hello-password] server-domain server-port (PUSH | PULL) (PERFECT | UPDATE) (COPY | MOVE) root-dir rel-dir

		time-margin-sec ... 同じ日時と見なす誤差の範囲 (秒) デフォルト 2 秒

		/TC  ... 作成日時を比較する。
		/TCW ... 作成日時と更新日時を比較する。

		デフォルトでは更新日時を比較する。
		★★★日時の比較は、新旧ではなく「同じかどうか」であることに注意！

		PUSH ... クライアント -> サーバー
		PULL ... サーバー -> クライアント

		PERFECT ... 元ディレクトリに存在しないファイル・ディレクトリを先ディレクトリから削除する。
		UPDATE  ... 元ディレクトリに存在しないファイル・ディレクトリを先ディレクトリから削除しない。

		COPY ... 同期するだけ
		MOVE ... 同期に成功したら、元ディレクトリの中身をクリアする。

		root-dir は存在するディレクトリでなければならない。
		rel-dir は（サーバー側にも）存在するディレクトリでなければならない！
*/

#include "C:\Factory\Common\Options\SClient.h"
#include "libs\nSyncCommon.h"
#include "libs\nSyncDefine.h"

enum
{
	TIME_CREATE = 0x01,
	TIME_WRITE  = 0x02,
};

static uint TimeCompMode = TIME_WRITE;
static time_t TimeMarginSec = 2;
static char *HelloPassword;

static char *ServerDomain;
static uint ServerPort;

static int PushMode;
static int PerfectMode;
static int MoveMode;

static char *RootDir;
static char *RelDir;
static char *ActiveDir;

static autoList_t *ClientDirs;
static autoList_t *ServerDirs;
static autoList_t *BothDirs;

static autoList_t *ClientFiles;
static autoList_t *ServerFiles;
static autoList_t *BothFiles;

static int CheckRecv(SockStream_t *ss, char *expect)
{
	char *line = SockRecvLine(ss, strlen(expect) + 1);
	int ret;

	ret = !strcmp(line, expect);

	memFree(line);
	return ret;
}
static int CheckEcho(SockStream_t *ss)
{
	int ret;

	SockSendLine(ss, ECHO_WORD_REQ);
	ret = CheckRecv(ss, ECHO_WORD_ANS);

	if (!ret)
		cout("fault echo!\n");

	return ret;
}
static autoList_t *RecvLines(SockStream_t *ss)
{
	autoList_t *lines = newList();

	for (; ; )
	{
		char *line = SockRecvLine(ss, RECV_LINE_LENMAX);

		if (!*line)
		{
			memFree(line);
			break;
		}
		addElement(lines, (uint)line);
	}
	return lines;
}
static int IsSameStamp(uint64 stamp1, uint64 stamp2)
{
	time_t t1 = getTimeByFileStamp(stamp1);
	time_t t2 = getTimeByFileStamp(stamp2);

	return _abs64(t1 - t2) <= TimeMarginSec;
}
static int IsSameFileStamp(uint64 createStamp1, uint64 writeStamp1, uint64 createStamp2, uint64 writeStamp2)
{
	int ret;

	cout("<CWS.1: %I64u %I64u\n", createStamp1, writeStamp1);
	cout("<CWS.2: %I64u %I64u\n", createStamp2, writeStamp2);

	ret =
		(!(TimeCompMode & TIME_CREATE) || IsSameStamp(createStamp1, createStamp2)) &&
		(!(TimeCompMode & TIME_WRITE)  || IsSameStamp(writeStamp1,  writeStamp2));

	cout("> %d\n", ret);

	return ret;
}
static void NSC_SendFile(SockStream_t *ss, char *file)
{
	SockSendLine(ss, "Send");
	SockSendLine(ss, file);
	NS_SendFile(ss, file);
}
static void NSC_RecvFile(SockStream_t *ss, char *file)
{
	SockSendLine(ss, "Recv");
	SockSendLine(ss, file);
	NS_RecvFile(ss, file);
}
static int Perform(int sock, uint dummyPrm)
{
	SockStream_t *ss = CreateSockStream(sock, 2);
	char *dir;
	char *file;
	uint index;
	int ret = 0;

	LOGPOS();

	// 最初のエコーのやり取りが成功するまでは、短いタイムアウトを設定しておく。
	// それ以降は無制限

	if (HelloPassword)
		SockSendLine(ss, HelloPassword);

	LOGPOS();

	if (!CheckEcho(ss))
	{
		LOGPOS();
		goto endFunc;
	}
	LOGPOS();

	SetSockStreamTimeout(ss, 0);

	ClientDirs = lssDirs(ActiveDir);
	changeRoots(ClientDirs, ActiveDir, NULL);

	ClientFiles = lssFiles(ActiveDir);
	changeRoots(ClientFiles, ActiveDir, NULL);

	SockSendLine(ss, "Start");
	SockSendLine(ss, RelDir);

	ServerDirs  = RecvLines(ss);
	ServerFiles = RecvLines(ss);

	if (!CheckEcho(ss))
		goto endFunc;

	BothDirs  = merge(ClientDirs,  ServerDirs,  (sint (*)(uint, uint))mbs_stricmp, (void (*)(uint))memFree);
	BothFiles = merge(ClientFiles, ServerFiles, (sint (*)(uint, uint))mbs_stricmp, (void (*)(uint))memFree);

	addCwd(ActiveDir); // ここから ActiveDir に入るので、相対パスでファイル・ディレクトリ操作OK!

	foreach (ClientDirs, dir, index)
	{
		cout("CD %s\n", dir);

		if (PushMode) // PUSH
		{
			cout("SEND-MD\n");
			SockSendLine(ss, "MD");
			SockSendLine(ss, dir);
		}
		else // PULL
		{
			if (PerfectMode)
			{
				cout("DEL-DIR\n");
				NS_DeletePath(dir);
			}
		}
	}
	foreach (ServerDirs, dir, index)
	{
		cout("SD %s\n", dir);

		if (PushMode) // PUSH
		{
			if (PerfectMode)
			{
				cout("SEND-DEL-DIR\n");
				SockSendLine(ss, "Delete");
				SockSendLine(ss, dir);
			}
		}
		else // PULL
		{
			cout("MD\n");
			NS_CreateParent(dir);
			createDir(dir);
		}
	}
	foreach (BothDirs, dir, index)
	{
		cout("BD %s\n", dir);
	}
	foreach (ClientFiles, file, index)
	{
		cout("CF %s\n", file);

		if (PushMode) // PUSH
		{
			NSC_SendFile(ss, file);
		}
		else // PULL
		{
			if (PerfectMode)
			{
				cout("DEL-FILE\n");
				NS_DeletePath(file);
			}
		}
	}
	foreach (ServerFiles, file, index)
	{
		cout("SF %s\n", file);

		if (PushMode) // PUSH
		{
			if (PerfectMode)
			{
				cout("SEND-DEL-FILE\n");
				SockSendLine(ss, "Delete");
				SockSendLine(ss, file);
			}
		}
		else // PULL
		{
			NSC_RecvFile(ss, file);
		}
	}
	foreach (BothFiles, file, index)
	{
		uint64 serverCreateStamp;
		uint64 serverWriteStamp;
		uint64 clientCreateStamp;
		uint64 clientWriteStamp;

		cout("BF %s\n", file);

		SockSendLine(ss, "GetFileStamp");
		SockSendLine(ss, file);

		serverCreateStamp = SockRecvValue64(ss);
		serverWriteStamp  = SockRecvValue64(ss);

		if (!CheckEcho(ss))
			break;

		getFileStamp(file, &clientCreateStamp, NULL, &clientWriteStamp);

		if (!IsSameFileStamp(serverCreateStamp, serverWriteStamp, clientCreateStamp, clientWriteStamp))
		{
			if (PushMode) // PUSH
			{
				NSC_SendFile(ss, file);
			}
			else // PULL
			{
				NSC_RecvFile(ss, file);
			}
		}
	}

	unaddCwd();

	if (!CheckEcho(ss))
		goto endFunc;

	if (MoveMode)
	{
		if (PushMode) // PUSH
		{
			cout("CLEAR\n");
			recurClearDir(ActiveDir);
		}
		else // PULL
		{
			cout("SEND-CLEAR\n");
			SockSendLine(ss, "Clear");
		}
	}

	ret = CheckEcho(ss);

endFunc:
	ReleaseSockStream(ss);

	return ret;
}

static uint ProcMtx;

static void UnlockProcMtx(void)
{
	LOGPOS();
	mutexUnlock(ProcMtx);
	LOGPOS();
}
static void LockProcMtx(void)
{
	LOGPOS();
	ProcMtx = mutexLock("{8f16d7a8-fe3e-4450-9322-db05b58903aa}");
	LOGPOS();
	addFinalizer(UnlockProcMtx);
	LOGPOS();
}
static void Main2(void)
{
	int errorLevel = 0;

	LockProcMtx(); // たまに同時に動かしてしまうことがあるので、ロック

readArgs:
	if (argIs("/T"))
	{
		TimeMarginSec = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/TC"))
	{
		TimeCompMode = TIME_CREATE;
		goto readArgs;
	}
	if (argIs("/TCW"))
	{
		TimeCompMode = TIME_CREATE | TIME_WRITE;
		goto readArgs;
	}
	if (argIs("/HPW"))
	{
		HelloPassword = nextArg();
		goto readArgs;
	}

	ServerDomain = nextArg();
	ServerPort = toValue(nextArg());

	if (argIs("PUSH"))
		PushMode = 1;
	else if (argIs("PULL"))
		PushMode = 0;
	else
		error();

	if (argIs("PERFECT"))
		PerfectMode = 1;
	else if (argIs("UPDATE"))
		PerfectMode = 0;
	else
		error();

	if (argIs("COPY"))
		MoveMode = 0;
	else if (argIs("MOVE"))
		MoveMode = 1;
	else
		error();

	RootDir = nextArg();
	RelDir = nextArg();

	// ---- Check args とか ----

	errorCase(!*ServerDomain);
	errorCase(!m_isRange(ServerPort, 1, 65535));

	// PushMode
	// PerfectMode
	// MoveMode

	errorCase(!*RootDir);
	errorCase(!existDir(RootDir));
	errorCase(!*RelDir);

	ActiveDir = combine(RootDir, RelDir);

	errorCase(!existDir(ActiveDir));

	// ----

	RootDir   = makeFullPath(RootDir);
	ActiveDir = makeFullPath_x(ActiveDir);

	cout("> %s:%u\n", ServerDomain, ServerPort);
	cout("%s\n", PushMode ? "PUSH" : "PULL");
	cout("PREFECT%c\n", PerfectMode ? '+' : '-');
	cout("%s\n", MoveMode ? "MOVE" : "COPY");
	cout("R %s\n", RootDir);
	cout("r %s\n", RelDir);
	cout("A %s\n", ActiveDir);

	NS_AppTitle = "nSync";
	cmdTitle(NS_AppTitle);

	if (!SClient(ServerDomain, ServerPort, Perform, 0))
	{
#if 0
		error_m("同期に失敗しました。");
#else
		// errorだとびっくりするので、こっちでいいや。@ 2017.7.4
		cout("+---------------------+\n");
		cout("| 同期に失敗しました。|\n");
		cout("+---------------------+\n");
		sleep(3000);
#endif
		errorLevel = 1;
	}
	cmdTitle(NS_AppTitle);

	cout("errorLevel: %u\n", errorLevel);
	termination(errorLevel);
}
int main(int argc, char **argv)
{
	Main2();
	termination(0); // 2bs
}
