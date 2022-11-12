/*
	C:\Factory\Common\all.h より先に SockServer.h をインクルードするか all.h のインクルードを省略すること。

	呼び出しシーケンス(１プロセス):
		LISTEN | STARTUP, LISTENS, CLEANUP
	LISTENS:
		LISTEN, LISTENS | ""
	STARTUP:
		SockStartup()
	LISTEN:
		sockServer() | sockServerEx() | sockServerUserTransmit()
	CLEANUP:
		SockCleanup()

	----

	最悪のメモリ使用量 (*1)
		sockServer()
			最大接続数 * (SockFile_t バッファサイズ @ SockRecvFile, SockSendFile) ==
			connectmax * (1024 * 1024) [byte]

		sockServerUserTransmit()
			0 [byte]

	最悪のディスク使用量 (*1)
		sockServer()
			最大接続数 * (受信データ最大サイズ + (応答データ最大サイズ == funcPerform() の出力するファイルの最大サイズ)) ==
			connectmax * (uploadmax + 応答データ最大サイズ)

		sockServerUserTransmit()
			0 [byte]

	(*1) システム側・コールバック側で消費する分、スタックに積まれる分など細かいものは考慮していない。

	コールバックも考慮した最悪のメモリ・ディスク使用量
		コールバックとして funcPerform, funcTransmit, funcCreateUserInfo, funcReleaseUserInfo, funcIdle がある。
		それぞれの呼び出しのために大規模に展開されるファイル・メモリブロックは無く、全てアトミックに実行されるので、
		単純に「最悪のメモリ・ディスク使用量」にコールバックでの最悪の使用量を加えたものとする。
*/

#include "SockServer.h"

#define MILLIS_TIMEOUT_SELECT_ACCEPT 100
//#define MILLIS_TIMEOUT_SELECT_SEND_RECV 0
//#define MILLIS_TIMEOUT_SELECT_SEND_RECV 1
#define MILLIS_TIMEOUT_SELECT_SEND_RECV 100
#define SECOND_TIMEOUT_TRANSMISSION 3600

typedef struct Transmission_st
{
	int Death;
	int Sock;
	uchar ClientIp[4];
	SockBlock_t *Signature;
	SockBlock_t *PrmSize;
	SockFile_t *PrmFile;
	int PerformComplete;
	SockBlock_t *AnsSize;
	SockFile_t *AnsFile;
	SockBlock_t *AnsAck;
	time_t ConnectedTime;
	uint UserInfo;
	uint UserInfoInited;
}
Transmission_t;

static autoList_t *Transmissions;
static int InterruptMode;

static int (*FuncPerform)(char *, char *);
static int (*FuncTransmit)(int, uint);
static uint (*FuncCreateUserInfo)(void);
static void (*FuncReleaseUserInfo)(uint);
static uint64 PrmFileSizeMax;

uchar sockClientIp[4];
uint sockUserTransmitIndex;

static Transmission_t *CreateTransmission(int sock, char *strip)
{
	Transmission_t *i = (Transmission_t *)memAlloc(sizeof(Transmission_t));

	i->Death = 0;
	i->Sock = sock;
	SockLine2Ip(i->ClientIp, strip);
	i->Signature = SockCreateBlock(strlen(SOCK_SIGNATURE));
	i->PrmSize = SockCreateBlock(sizeof(uint64));
	i->PrmFile = SockCreateFile(makeTempFile("sock-prm"), 0);
	i->PerformComplete = 0;
	i->AnsSize = SockCreateBlock(sizeof(uint64));
	i->AnsFile = SockCreateFile(makeTempFile("sock-ans"), 0);
	i->AnsAck = SockCreateBlock(1);
	i->ConnectedTime = SockCurrTime;
	i->UserInfo = 0;
	i->UserInfoInited = 0;

	return i;
}
static Transmission_t *CreateTransmissionEx(int sock)
{
	struct sockaddr_in clsa;
	int sasz;
	int clsock;

	sasz = sizeof(clsa);
	clsock = accept(sock, (struct sockaddr *)&clsa, &sasz);
	errorCase(clsock == -1);
	SockPostOpen(clsock);

	sockConnectedCount++;
	sockConnectMonUpdated();

	return CreateTransmission(clsock, inet_ntoa(clsa.sin_addr));
}
static void ReleaseTransmission(Transmission_t *i)
{
	SockPreClose(i->Sock);
	shutdown(i->Sock, SD_BOTH);
	closesocket(i->Sock);

	sockConnectedCount--;
	sockConnectMonUpdated();

	SockReleaseBlock(i->Signature);
	SockReleaseBlock(i->PrmSize);
	SockReleaseFile(i->PrmFile);
	SockReleaseBlock(i->AnsSize);
	SockReleaseFile(i->AnsFile);
	SockReleaseBlock(i->AnsAck);

	if (i->UserInfoInited)
		FuncReleaseUserInfo(i->UserInfo);

	memFree(i);
}
static void PerformTransmit(Transmission_t *t, uint waitMillis)
{
	if (t->Signature->Counter < t->Signature->BlockSize)
	{
		t->Death = SockTransmitBlock(t->Sock, t->Signature, waitMillis, 0) == -1;

		if (t->Death) return;
		if (t->Signature->Counter < t->Signature->BlockSize) return;

		t->Death = memcmp(t->Signature->Block, SOCK_SIGNATURE, t->Signature->BlockSize); // ? 不正なシグネチャ

		if (t->Death) return;
	}
	if (t->PrmSize->Counter < t->PrmSize->BlockSize)
	{
		uint64 size;

		t->Death = SockTransmitBlock(t->Sock, t->PrmSize, waitMillis, 0) == -1;

		if (t->Death) return;
		if (t->PrmSize->Counter < t->PrmSize->BlockSize) return;

		size = blockToValue64(t->PrmSize->Block);

		if (PrmFileSizeMax < size)
		{
			cout("+----------------------------------------------+\n");
			cout("| 受信データファイルサイズが制限を超えています |\n");
			cout("| memo: uploadmax が小さすぎる？               |\n");
			cout("+----------------------------------------------+\n");
			cout("PrmFileSizeMax: %I64u\n", PrmFileSizeMax);
			cout("size: %I64u\n", size);

			t->Death = 1;
			return;
		}
		t->PrmFile->FileSize = size;
	}
	if (t->PrmFile->Counter < t->PrmFile->FileSize)
	{
		t->Death = SockRecvFile(t->Sock, t->PrmFile, waitMillis) == -1;

		if (t->Death) return;
		if (t->PrmFile->Counter < t->PrmFile->FileSize) return;
	}
	if (!t->PerformComplete)
	{
		uint64 fileSize;

		if (InterruptMode) // funcPerform() 再帰呼び出し防止
			return;

		*(uint *)sockClientIp = *(uint *)t->ClientIp;

		if (FuncPerform(t->PrmFile->File, t->AnsFile->File) == 0) // ? エラーまたは失敗
		{
			t->Death = 1;
			return;
		}
		t->PerformComplete = 1;

		fileSize = getFileSize(t->AnsFile->File);
		value64ToBlock(t->AnsSize->Block, fileSize);
		t->AnsFile->FileSize = fileSize;
	}
	if (t->AnsSize->Counter < t->AnsSize->BlockSize)
	{
		t->Death = SockTransmitBlock(t->Sock, t->AnsSize, waitMillis, 1) == -1;

		if (t->Death) return;
		if (t->AnsSize->Counter < t->AnsSize->BlockSize) return;
	}
	if (t->AnsFile->Counter < t->AnsFile->FileSize)
	{
		t->Death = SockSendFile(t->Sock, t->AnsFile, waitMillis) == -1;

		if (t->Death) return;
		if (t->AnsFile->Counter < t->AnsFile->FileSize) return;
	}
	if (t->AnsAck->Counter < t->AnsAck->Counter)
	{
		t->Death = SockTransmitBlock(t->Sock, t->AnsAck, waitMillis, 0) == -1; // ここで死んでも正常

		if (t->Death) return;
		if (t->AnsAck->Counter < t->AnsAck->BlockSize) return;
	}
	t->Death = 1;
}
static void Transmission(void)
{
	Transmission_t *t;
	uint index;

	foreach (Transmissions, t, index)
	{
		if (FuncTransmit)
		{
			errorCase(!t);

			*(uint *)sockClientIp = *(uint *)t->ClientIp;
			sockUserTransmitIndex = index;

			if (!t->UserInfoInited)
			{
				t->UserInfoInited = 1;
				t->UserInfo = FuncCreateUserInfo();
			}
			t->Death = FuncTransmit(t->Sock, t->UserInfo) == 0;
		}
		else
		{
			if (!t)
				continue; // ここへ到達するケース：切断があってから sockServerPerformInterrupt() から再帰的に呼び出された。

			PerformTransmit(t, index || InterruptMode ? 0 : MILLIS_TIMEOUT_SELECT_SEND_RECV);

			if (t->ConnectedTime + SECOND_TIMEOUT_TRANSMISSION < SockCurrTime)
			{
				t->Death = 1;
			}
		}
		if (t->Death)
		{
			ReleaseTransmission(t);
			setElement(Transmissions, index, 0);
		}
	}
}

static int SSPISock;
static uint SSPIConnectMax;

/*
	sockServer(), sockServerEx() における funcPerform() の１回の実行は数秒で終わることを想定している。
	数分掛かったりすると他の接続が SockTransmitBlock() のタイムアウトで切断されてしまうだろう。
	これを避けるため、長時間掛かる funcPerform() の実行中は定期的に sockServerPerformInterrupt() を呼ぶこと。
*/
void sockServerPerformInterrupt(void)
{
	uint index = findElement(Transmissions, 0, simpleComp);

	if (index < SSPIConnectMax)
		if (SockWait(SSPISock, 0, 0)) // ? 接続あり
			putElement(Transmissions, index, (uint)CreateTransmissionEx(SSPISock));

	InterruptMode = 1; // PerformTransmit() から再帰的に funcPerform を呼び出さないように
	Transmission();
	InterruptMode = 0;
}

/*
	int funcPerform(char *prmFile, char *ansFile);
		RPCぽい通信を行う。
		sockClientIp が有効

		prmFile
			要求データを記述したバイナリファイル、変更してもよいが、削除してはならない。(本関数の終了時に存在すれば良い)

		ansFile
			応答データを記述するバイナリファイル
			本関数を開始した時点では 0 バイトのファイルが存在する。
			本関数を終了する前に、応答データを書き出すこと。(エラーの場合でも) 削除してはならない。(本関数の終了時に存在すれば良い)
			応答データは 0 ～ UINT64MAX バイトを想定しているつもり。

		戻り値
			0 以外 == 正常終了
			0      == エラーまたは失敗、すぐに接続を切断する。

		prmFile, ansFile 共に誤り検出・訂正を行わない。-> PadFile.c を使うように..

	void funcTransmit(int sock, uint userInfo);
		呼び出し側の方法で通信を行う。
		funcTransmit(), funcCreateUserInfo(), funcReleaseUserInfo() 何れの呼び出し中も sockClientIp が有効

		sock
			接続中のソケット、funcTransmit() 内で shutdown(), closesocket() してはならない。

		userInfo
			uint funcCreateUserInfo(void); の戻り値
			切断時に void funcReleaseUserInfo(uint userInfo); に渡す。

		戻り値
			0 以外 == 正常終了
			0      == エラーまたは通信終了、すぐに接続を切断する。

	portno - ポート番号 (1～65535)
	connectmax - 最大同時接続数、せいぜい 10～20 くらいにしておいた方が良いんじゃないだろうか
	uploadmax - 要求データの最大サイズ

	最悪のメモリ使用量: connectmax * 1024^2 [byte] (*1)
	最悪のディスク使用量: connectmax * (uploadmax + 応答データ最大サイズ) [byte] (*1)

	(*1) 管理領域とか細かいところは考慮しない。

	int funcIdle(void);
		不定期に何度も呼び出す。

		戻り値
			0 以外 == サーバーを続行する。
			0      == サーバーを停止する。
*/
void sockServerEx(int (*funcPerform)(char *, char *),
	int (*funcTransmit)(int, uint),
	uint (*funcCreateUserInfo)(void),
	void (*funcReleaseUserInfo)(uint),
	uint portno, uint connectmax, uint64 uploadmax, int (*funcIdle)(void))
{
	int retval;
	int sock;
	struct sockaddr_in sa;
	Transmission_t *t;
	uint index;

	errorCase( funcPerform && ( funcTransmit ||  funcCreateUserInfo ||  funcReleaseUserInfo));
	errorCase(!funcPerform && (!funcTransmit || !funcCreateUserInfo || !funcReleaseUserInfo));
	errorCase(!portno || 0xffff < portno);
	errorCase(!connectmax);
//	errorCase(uploadmax < 0);
	errorCase(!funcIdle);

	SockStartup();

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	errorCase(sock == -1);
	SockPostOpen(sock);

	memset(&sa, 0x00, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons((unsigned short)portno);

	retval = bind(sock, (struct sockaddr *)&sa, sizeof(sa));
	errorCase_m(retval != 0, "Can not bind to TCP port."); // ? == -1

	retval = listen(sock, SOMAXCONN);
	errorCase(retval != 0); // ? == -1

	Transmissions = newList();

	FuncPerform = funcPerform;
	FuncTransmit = funcTransmit;
	FuncCreateUserInfo = funcCreateUserInfo;
	FuncReleaseUserInfo = funcReleaseUserInfo;
	PrmFileSizeMax = uploadmax;

	SSPISock = sock;
	SSPIConnectMax = connectmax;

	sockConnectMax = connectmax;
	sockConnectMonUpdated();

	sockServerMode = 1;

	for (; ; )
	{
		static uint xcRchd;

		SockCurrTime = time(NULL);

		if (funcIdle() == 0) // ? サーバー停止
			break;

		Transmission();
		removeZero(Transmissions);

		if (getCount(Transmissions) < connectmax)
		{
			if (SockWait(sock, getCount(Transmissions) ? 0 : MILLIS_TIMEOUT_SELECT_ACCEPT, 0)) // ? 接続あり
				addElement(Transmissions, (uint)CreateTransmissionEx(sock));

			xcRchd = 0;
		}
		else if (!xcRchd)
		{
			cout("Reached the maximum number of connections.\n");
			xcRchd = 1;
		}
	}

	foreach (Transmissions, t, index)
		ReleaseTransmission(t);

	releaseAutoList(Transmissions);

	SockPreClose(sock);
	retval = closesocket(sock);
	errorCase(retval);

	SockCleanup();
}
void sockServer(int (*funcPerform)(char *, char *), uint portno, uint connectmax, uint64 uploadmax, int (*funcIdle)(void))
{
	sockServerEx(funcPerform, NULL, NULL, NULL, portno, connectmax, uploadmax, funcIdle);
}
void sockServerUserTransmit(
	int (*funcTransmit)(int, uint),
	uint (*funcCreateUserInfo)(void),
	void (*funcReleaseUserInfo)(uint),
	uint portno, uint connectmax, int (*funcIdle)(void))
{
	sockServerEx(NULL, funcTransmit, funcCreateUserInfo, funcReleaseUserInfo, portno, connectmax, 0ui64, funcIdle);
}
