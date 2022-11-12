/*
	C:\Factory\Common\all.h より先に SockClient.h をインクルードするか all.h のインクルードを省略すること。

	呼び出しシーケンス(１プロセス):
		CONNECT | STARTUP, CONNECTS, CLEANUP
	CONNECTS:
		CONNECT, CONNECTS | ""
	STARTUP:
		SockStartup()
	CONNECT:
		sockClient() | sockClientEx() | sockClientUserTransmit()
	CLEANUP:
		SockCleanup()

	- - -

	★★★ 今後は SClient() を使うようにしてね。
*/

#include "SockClient.h"

#define MILLIS_TIMEOUT_TRANSMIT 300
#define SEC_IP_TIMEOUT 180

/*
	(先方が)ファイアウォールを有効にしていると gethostbyname() で IP が取れない。
	「ファイルとプリンタの共有」を例外にすると取得できるようになる。
	ping に応答するかしないかが関係？
*/
autoList_t *sockLookupList(char *domain)
{
	autoList_t *ips = newList();
	struct hostent *host;
	struct in_addr addr;
	uint index;
	uint ip;

	SockStartup();
	host = gethostbyname(domain); // domain == "" -> 自分のIP。複数のNICを挿していると、複数返る。

	if (host)
	{
		for (index = 0; host->h_addr_list[index]; index++)
		{
			addr.s_addr = *(u_long *)host->h_addr_list[index];
			SockLine2Ip((uchar *)&ip, inet_ntoa(addr));
			addElement(ips, ip);
		}
	}
	SockCleanup();
	return ips;
}
/*
	ip, domain: NULL 不可
	domain に対する最初のIPアドレスを ip に格納する。
	見つからないときは 0.0.0.0 を格納する。
	domain == "" -> 自分のIP
*/
void sockLookup(uchar ip[4], char *domain)
{
	autoList_t *ips = sockLookupList(domain);

	*(uint *)ip = refElement(ips, 0);
	releaseAutoList(ips);
}

static uchar *GetDefIP(char *domain)
{
	static uchar ip[4];

	memset(ip, 0x00, 4);
	return ip;
}
static int IsTimeout(uchar ip[4])
{
	static autoList_t *uipList;
	static autoList_t *tmoutList;
	uint uip = *(uint *)ip;
	uint uipPos;

	if (!uipList)
	{
//		LOGPOS(); // test
		uipList = newList();
		tmoutList = newList();
	}
	uipPos = findElement(uipList, uip, simpleComp);

	if (uipPos == getCount(uipList))
	{
//		LOGPOS(); // test
		addElement(uipList, uip);
		addElement(tmoutList, now() + SEC_IP_TIMEOUT);
		return 0;
	}
	if (getElement(tmoutList, uipPos) < now())
	{
//		LOGPOS(); // test
		fastDesertElement(uipList, uipPos);
		fastDesertElement(tmoutList, uipPos);
		return 1;
	}
	return 0;
}

static int ConnectWithTimeout(int sock, struct sockaddr *p_sa, uint timeoutMillis, int nonBlocking, int *p_timedOut)
{
	WSAEVENT ev;
	WSANETWORKEVENTS nwEv;
	int retval = -1;
	int ret;
	int ioctlsocket_arg;

	ev = WSACreateEvent();

	if (ev == WSA_INVALID_EVENT)
		goto endfunc;

//LOGPOS(); // test
	ret = WSAEventSelect(sock, ev, FD_CONNECT);
//cout("ret: %d\n", ret); // test
//LOGPOS(); // test

	if (ret == -1)
		goto endfunc_ev;

//LOGPOS(); // test
	ret = connect(sock, p_sa, sizeof(*p_sa));
//cout("ret: %d\n", ret); // test
//LOGPOS(); // test

	if (ret == -1)
	{
//LOGPOS(); // test
		ret = WSAGetLastError();
//cout("ret: %d\n", ret); // test
//LOGPOS(); // test

		if (ret != WSAEWOULDBLOCK)
			goto endfunc_nwEv;
	}
//LOGPOS(); // test
	if (nonBlocking)
		inner_uncritical();

	ret = WSAWaitForMultipleEvents(1, &ev, 0, timeoutMillis, 0);

	if (nonBlocking)
		inner_critical();
//cout("ret: %d\n", ret); // test
//LOGPOS(); // test

	// https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-wsawaitformultipleevents

	// WSA_WAIT_TIMEOUT        <---- タイムアウト
	// or
	// WSA_WAIT_IO_COMPLETION  <---- This return value can only be returned if the fAlertable(第5引数) parameter is TRUE.
	// or
	// WSA_WAIT_EVENT_0 ～ (WSA_WAIT_EVENT_0 + cEvents - 1)

	// WSA_WAIT_EVENT_0       == 0
	// WSA_WAIT_IO_COMPLETION == 192 (0xC0)
	// WSA_WAIT_TIMEOUT       == 258

	if (ret != WSA_WAIT_EVENT_0)
	{
		*p_timedOut = 1;
		goto endfunc_nwEv;
	}
//LOGPOS(); // test
	ret = WSAEnumNetworkEvents(sock, ev, &nwEv);
//cout("ret: %d\n", ret); // test
//LOGPOS(); // test

	if (
		ret == -1 ||
		!(nwEv.lNetworkEvents & FD_CONNECT) ||
		nwEv.iErrorCode[FD_CONNECT_BIT] != 0
		)
		goto endfunc_nwEv;

	retval = 0;

	ioctlsocket_arg = 0;
	ioctlsocket(sock, FIONBIO, &ioctlsocket_arg);

//LOGPOS(); // test
endfunc_nwEv:
//LOGPOS(); // test
	WSAEventSelect(sock, NULL, 0);
endfunc_ev:
//LOGPOS(); // test
	WSACloseEvent(ev);
endfunc:
//LOGPOS(); // test
	return retval;
}

/*
	簡単な使用例
		SockStartup();
		sock = sockConnect(ip, domain, portno);
		if (sock != -1) {
			ここで通信
			sockDisconnect(sock);
		}
		SockCleanup();
*/
int sockConnectEx(uchar ip[4], char *domain, uint portno, int nonBlocking) // ret: -1 == 失敗, -1 以外 == 成功, 成功のときは sockDisconnect() を忘れずに
{
	char *strip;
	int sock;
	struct sockaddr_in sa;
	int retval;

	if (!ip)
		ip = GetDefIP(domain);

	if (!*(uint *)ip) // ? 0.0.0.0
	{
		errorCase(!domain);
		sockLookup(ip, domain);

		if (!*(uint *)ip) return -1;
	}
	strip = SockIp2Line(ip);

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	errorCase(sock == -1);
	SockPostOpen(sock);

	memset(&sa, 0x00, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(strip);
	sa.sin_port = htons((unsigned short)portno);

	errorCase(sa.sin_addr.s_addr == INADDR_NONE);

	{
		uint timeoutMillis = sockConnectTimeoutSec * 1000;
		int timedOut = 0;

#if 1
		retval = ConnectWithTimeout(sock, (struct sockaddr *)&sa, timeoutMillis, nonBlocking, &timedOut);
#else // old @ 2018.8.20
		if (nonBlocking)
			inner_uncritical();

		retval = connect(sock, (struct sockaddr *)&sa, sizeof(sa));

		if (nonBlocking)
			inner_critical();
#endif

		if (timedOut)
		{
			int warning = !nonBlocking && sockServerMode;

			cout("%s: アクティブオープンがタイムアウトしました。%u[ms] %s %s %d\n",
				warning ? "Warning" : "Information",
				timeoutMillis,
				nonBlocking ? "NB" : "B",
				sockServerMode ? "SVR" : "CL",
				retval
				);

			if (warning)
				setConsoleColor(0x5f);
		}
	}

	if (retval == -1) // ? 接続失敗
	{
		if (domain) // ? domain 有効
			*(uint *)ip = 0; // domain に対応する ip が更新されたかもしれない。-> 次回 ip 更新のため 0.0.0.0 再設定

		SockPreClose(sock);
		closesocket(sock);
		return -1;
	}
	if (IsTimeout(ip))
	{
		LOGPOS(); // test
		*(uint *)ip = 0; // 定期的にリセットする。
	}
	return sock;
}
int sockConnect(uchar ip[4], char *domain, uint portno)
{
	return sockConnectEx(ip, domain, portno, 0);
}
int sockConnect_NB(uchar ip[4], char *domain, uint portno)
{
	return sockConnectEx(ip, domain, portno, 1);
}
void sockDisconnect(int sock)
{
	SockPreClose(sock);
	shutdown(sock, SD_BOTH);
	closesocket(sock);
}

typedef struct Transmission_st
{
	SockBlock_t *Signature;
	SockBlock_t *PrmSize;
	SockFile_t *PrmFile;
	SockBlock_t *AnsSize;
	SockFile_t *AnsFile;
	SockBlock_t *AnsAck;
}
Transmission_t;

static Transmission_t *CreateTransmission(char *prmFile)
{
	Transmission_t *i = (Transmission_t *)memAlloc(sizeof(Transmission_t));

	i->Signature = SockCreateBlock(strlen(SOCK_SIGNATURE));
	i->PrmSize = SockCreateBlock(sizeof(uint64));
	i->PrmFile = SockCreateFile(prmFile, getFileSize(prmFile));
	i->AnsSize = SockCreateBlock(sizeof(uint64));
	i->AnsFile = SockCreateFile(makeTempFile("clsock-ans"), 0);
	i->AnsAck = SockCreateBlock(1);

	return i;
}
static void ReleaseTransmission(Transmission_t *i)
{
	i->PrmFile->File = makeTempFile("clsock-prm-dummy");

	SockReleaseBlock(i->Signature);
	SockReleaseBlock(i->PrmSize);
	SockReleaseFile(i->PrmFile);
	SockReleaseBlock(i->AnsSize);
	SockReleaseFile(i->AnsFile);
	SockReleaseBlock(i->AnsAck);

	memFree(i);
}

sockClientStatus_t sockClientStatus;
uint64 sockClientAnswerFileSizeMax = 0x80000000 + 1024; // 2GB + margin

/*
	ip - 接続先のIPアドレス、domain に接続するには 0.0.0.0 を設定しておく。
		sockLookup() によって更新される。
		接続できないとき、domain が有効(非NULL)ならば 0.0.0.0 に再設定される。
	domain - 接続先のドメイン名、特に無い場合は NULL を指定する。
	portno - 接続先のポート番号 (1 ～ 65535)
	prmFile - 要求データ、0 ～ UINT64MAX バイトの存在するファイルであること。

	int idleFunc(void);
		不定期に何回か呼び出す。

		戻り値
			0 以外 == 正常終了
			0      == 通信を中止して、sockClientEx() は NULL を返す。

	void transmitFunc(int sock);
		呼び出し側の方法で通信を行う。

		sock
			接続中のソケット、transmitFunc() 内で shutdown(), closesocket() してはならない。

	戻り値
		NULL 以外 == 正常終了、応答データを記述したバイナリファイル
		             ファイルの削除とファイル名の開放は呼び出し側の責任
		NULL      == 通信エラー

	prmFile, ansFile(戻り値) 共に誤り検出・訂正を行わない。-> PadFile.c
*/
char *sockClientEx(uchar ip[4], char *domain, uint portno, char *prmFile, int (*idleFunc)(void), void (*transmitFunc)(int))
{
	char *ansFile = NULL;
	int sock;
	Transmission_t *t;

	errorCase(!ip);
	errorCase(!portno || 0xffff < portno);
	errorCase(!transmitFunc && !prmFile);
	errorCase(!transmitFunc && !existFile(prmFile));
//	errorCase(!transmitFunc && UINT64MAX < getFileSize(prmFile));
	errorCase( idleFunc &&  transmitFunc);
	errorCase(!idleFunc && !transmitFunc);

	SockStartup();
	sock = sockConnect(ip, domain, portno);

	if (sock == -1)
		goto error_connect;

	if (transmitFunc)
	{
		transmitFunc(sock);
		goto end_transmission;
	}
	t = CreateTransmission(prmFile);
	value64ToBlock(t->PrmSize->Block, t->PrmFile->FileSize);
	memcpy(t->Signature->Block, SOCK_SIGNATURE, strlen(SOCK_SIGNATURE));
	t->AnsAck->Block[0] = 0x00;

	sockClientStatus.PrmFile = t->PrmFile;
	sockClientStatus.AnsFile = t->AnsFile;

#define TransmitLoop(condLoop, condTransmitError) \
	while (condLoop) { \
		SockCurrTime = time(NULL); \
		if (idleFunc() == 0) goto error_transmission; \
		if (condTransmitError) goto error_transmission; \
	}

	TransmitLoop(
		t->Signature->Counter < t->Signature->BlockSize,
		SockTransmitBlock(sock, t->Signature, MILLIS_TIMEOUT_TRANSMIT, 1) == -1
		)
	TransmitLoop(
		t->PrmSize->Counter < t->PrmSize->BlockSize,
		SockTransmitBlock(sock, t->PrmSize, MILLIS_TIMEOUT_TRANSMIT, 1) == -1
		)
	TransmitLoop(
		t->PrmFile->Counter < t->PrmFile->FileSize,
		SockSendFile(sock, t->PrmFile, MILLIS_TIMEOUT_TRANSMIT) == -1
		)
	TransmitLoop(
		t->AnsSize->Counter < t->AnsSize->BlockSize,
		SockTransmitBlock_WF1B(sock, t->AnsSize, MILLIS_TIMEOUT_TRANSMIT, 0, 1) == -1
		)

	t->AnsFile->FileSize = blockToValue64(t->AnsSize->Block);

	if (sockClientAnswerFileSizeMax < t->AnsFile->FileSize)
	{
		cout("ERROR: sockClientAnswerFileSizeMax overflow. %I64u < %I64u\n", sockClientAnswerFileSizeMax, t->AnsFile->FileSize);
		goto error_transmission;
	}

	TransmitLoop(
		t->AnsFile->Counter < t->AnsFile->FileSize,
		SockRecvFile(sock, t->AnsFile, MILLIS_TIMEOUT_TRANSMIT) == -1
		)
	TransmitLoop(
		t->AnsAck->Counter < t->AnsAck->BlockSize,
		SockTransmitBlock(sock, t->AnsAck, MILLIS_TIMEOUT_TRANSMIT, 1) == -1
		)

#undef TransmitLoop

	// 通信成功
	ansFile = t->AnsFile->File;
	t->AnsFile->File = makeTempFile("clsock-ans-dummy");

error_transmission:
	ReleaseTransmission(t);
end_transmission:
	sockDisconnect(sock);
error_connect:
	SockCleanup();

	sockClientStatus.PrmFile = NULL;
	sockClientStatus.AnsFile = NULL;

	return ansFile;
}
char *sockClient(uchar ip[4], char *domain, uint portno, char *prmFile, int (*idleFunc)(void))
{
	return sockClientEx(ip, domain, portno, prmFile, idleFunc, NULL);
}
/*
	注意: 接続に失敗すると transmitFunc() は実行されずに終了する。
*/
void sockClientUserTransmit(uchar ip[4], char *domain, uint portno, void (*transmitFunc)(int))
{
	if (sockClientEx(ip, domain, portno, NULL, NULL, transmitFunc) != NULL)
	{
		error();
	}
}
