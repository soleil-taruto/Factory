/*
	boomServer.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	boomServer.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/SS SEND-SIZE-MAX] [/RS RECV-SIZE-MAX]
	                                           [/T SOCK-TIMEOUT] [/NCT NO-CONNECT-TIMEOUT]

		CONNECT-MAX   ... 最大同時接続数, 省略時は 50
		SEND-SIZE-MAX ... 最大送信データサイズ, [バイト] 最小21
		RECV-SIZE-MAX ... 最大受信データサイズ, [バイト] 最小21
		SOCK-TIMEOUT  ... １回の送受信のタイムアウト, [秒]
		NO-CONNECT-TIMEOUT ... 無接続タイムアウト, [秒]

		★転送先とのタイムアウトは無い。
*/

#include "Boomerang.h"
#include "libs\TimeWaitMonitor.h"
#include "C:\Factory\Common\Options\rbTree.h"
#include "C:\Factory\Common\Options\SockServer.h"

#define REAL_CONNECT_MAX ((uint)m_min((uint64)ConnectMax + 10, (uint64)UINTMAX))

static uint PortNo;
static char *FwdHost;
static uint FwdPortNo;

static uint ConnectMax = 50;
static uint SendSizeMax = 2500;
static uint RecvSizeMax = 2500;
static uint SockTimeoutSec = 20;
static uint NoConnectTimeoutSec = 60;

// ---- Session_t ----

typedef struct Session_st
{
	int Sock;
	Comm_t *Comm;
	int LastForegroundFlag;
	autoBlock_t *LastSendData;
	uint NoConnectTimeout;
}
Session_t;

static void UpdateNoConnectTimeout(Session_t *i)
{
	i->NoConnectTimeout = now() + NoConnectTimeoutSec;
}
static int IsNoConnectTimeout(Session_t *i)
{
	return i->NoConnectTimeout < now();
}

static Session_t *CreateSession(int sock)
{
	Session_t *i = nb_(Session_t);

	i->Sock = sock; // bind
	i->Comm = CreateComm(sock);
	i->LastSendData = newBlock();

	UpdateNoConnectTimeout(i);

	return i;
}
static void ReleaseSession(Session_t *i)
{
	ReleaseComm(i->Comm);
	sockDisconnect(i->Sock);
	releaseAutoBlock(i->LastSendData);
	memFree(i);
}

// ---- SessionTree ----

static uint ST_DuplKey(uint prm)
{
	return (uint)copyAutoBlock((autoBlock_t *)prm);
}
static sint ST_CompKey(uint prm1, uint prm2)
{
	autoBlock_t *key1 = (autoBlock_t *)prm1;
	autoBlock_t *key2 = (autoBlock_t *)prm2;

	errorCase(getSize(key1) != CREDENTIAL_SIZE);
	errorCase(getSize(key2) != CREDENTIAL_SIZE);

	return memcmp(directGetBuffer(key1), directGetBuffer(key2), CREDENTIAL_SIZE);
}
static void ST_ReleaseKey(uint prm)
{
	releaseAutoBlock((autoBlock_t *)prm);
}

static rbTree_t *SessionTree;

// ---- Boomerang ----

static autoBlock_t *RecvCredential;
static int RecvFlag;
static autoBlock_t *RecvData;

static void BoomerangRecv(SockStream_t *ss)
{
	uchar block[4];
	uint recvSize;
	uint crc16;
	uint r_crc16;

	if (!RecvCredential)
	{
		RecvCredential = nobCreateBlock(CREDENTIAL_SIZE);
		RecvData = newBlock();
	}
	SockRecvBlock(ss, block, 4);

	recvSize = blockToValue(block);

	if (recvSize < HEADER_SIZE || RecvSizeMax < recvSize)
		goto errorEnd;

	recvSize -= HEADER_SIZE;
	nobSetSize(RecvData, recvSize);

	SockRecvBlock(ss, directGetBuffer(RecvCredential), CREDENTIAL_SIZE);
	RecvFlag = SockRecvChar(ss);
	SockRecvChar(ss); // Reserved
	r_crc16 = SockRecvChar(ss);
	r_crc16 |= SockRecvChar(ss) << 8;
	SockRecvBlock(ss, directGetBuffer(RecvData), recvSize);

	crc16 = boom_GetCRC16(RecvCredential, RecvFlag, RecvData);

	cout("CRC16=%04x, R_CRC16=%04x\n", crc16, r_crc16);

	if (crc16 != r_crc16)
		goto errorEnd;

	return;
errorEnd:
	memset(directGetBuffer(RecvCredential), 0x00, CREDENTIAL_SIZE);
	RecvFlag = 'E'; // Error
	nobSetSize(RecvData, 0);
}
static void BoomerangSend(SockStream_t *ss, autoBlock_t *credential, int flag, autoBlock_t *sendData)
{
	uchar block[4];
	uint sendSize;
	uint crc16 = boom_GetCRC16(credential, flag, sendData);

	cout("CRC16=%04x\n", crc16);

	errorCase(getSize(credential) != CREDENTIAL_SIZE); // 2bs

	sendSize = HEADER_SIZE + getSize(sendData);
	valueToBlock(block, sendSize);

	SockSendBlock(ss, block, 4);
	SockSendBlock(ss, directGetBuffer(credential), CREDENTIAL_SIZE);
	SockSendChar(ss, flag);
	SockSendChar(ss, 0x00); // Reserved
	SockSendChar(ss, crc16 & 0xff);
	SockSendChar(ss, crc16 >> 8 & 0xff);
	SockSendBlock(ss, directGetBuffer(sendData), getSize(sendData));

	SockFlush(ss);
}

// ----

static void MakeCredential(uchar *buff)
{
	autoBlock_t *nc = makeCryptoRandBlock(CREDENTIAL_SIZE);

	memcpy(buff, directGetBuffer(nc), CREDENTIAL_SIZE);
	releaseAutoBlock(nc);
}

static autoBlock_t *SendCredential;
static int SendFlag;
static autoBlock_t *SendData;

static int Perform(int sock, uint prm)
{
	SockStream_t *ss = *(SockStream_t **)prm;
	Session_t *session;

	if (!ss)
	{
		ss = CreateSockStream(sock, SockTimeoutSec);
		*(SockStream_t **)prm = ss;
	}
	if (!SockRecvCharWait(ss, 100))
		return 1;

	BoomerangRecv(ss);

	if (RecvFlag == 'E') // Error
		goto errorEnd;

	// init
	{
		if (!SendCredential)
		{
			SendCredential = nobCreateBlock(CREDENTIAL_SIZE);
			SendData = newBlock();
		}
		memset(directGetBuffer(SendCredential), 0x00, CREDENTIAL_SIZE);
		SendFlag = 0x00;
		nobSetSize(SendData, 0);
	}

	if (!rbtHasKey(SessionTree, (uint)RecvCredential))
	{
		static uchar ip[4];
		int fwdSock;

		if (RecvFlag == 'D') // Disconnect
		{
			cout("Information: 存在しないセッションの切断要求を受信しました。\n");
setConsoleColor(0x0e); // test
			SendFlag = 'D'; // Disconnect
			goto doSend;
		}
		if (ConnectMax < rbtGetCount(SessionTree))
		{
			LOGPOS();
			SendFlag = 'E'; // Error
			goto doSend;
		}
		fwdSock = sockConnect(ip, FwdHost, FwdPortNo);

		if (fwdSock == -1)
		{
			LOGPOS();
			SendFlag = 'D'; // Disconnect
			goto doSend;
		}
		cout("[connect]\n");
		session = CreateSession(fwdSock);
		rbtAddValue(SessionTree, (uint)RecvCredential, (uint)session);
	}
	else
	{
		session = (Session_t *)rbtGetLastAccessValue(SessionTree);
	}

	if (RecvFlag == 'D') // Disconnect
	{
		LOGPOS();
		rbtUnaddLastAccessValue(SessionTree);
		ReleaseSession(session);
		SendFlag = 'D'; // Disconnect
		goto doSend;
	}
	UpdateNoConnectTimeout(session);

	if (RecvFlag == (session->LastForegroundFlag ? 'F' : 'B'))
	{
		LOGPOS();
		addBytes(SendData, session->LastSendData);
		goto doSend;
	}
	if (!AddCommSendData(session->Comm, RecvData, 0)) // ? 失敗
	{
		LOGPOS();
		SendFlag = 'E'; // Error
		goto doSend;
	}
	ab_addBytes_x(SendData, GetCommRecvData(session->Comm, SendSizeMax));

	if (IsCommDeadAndEmpty(session->Comm) && !getSize(SendData))
	{
		LOGPOS();
		rbtUnaddLastAccessValue(SessionTree);
		ReleaseSession(session);
		SendFlag = 'D'; // Disconnect
		goto doSend;
	}
	session->LastForegroundFlag ^= 1;
	LOGPOS();

doSend:
	ab_swap(RecvCredential, SendCredential);
	BoomerangSend(ss, SendCredential, SendFlag, SendData);

errorEnd:
	AddTimeWait();
	return 0;
}
static uint CreateInfo(void)
{
	return (uint)nb_(SockStream_t *);
}
static void ReleaseInfo(uint prm)
{
	SockStream_t *ss = *(SockStream_t **)prm;

	if (ss)
		ReleaseSockStream(ss);

	memFree((SockStream_t **)prm);
}

// ---- stop event ----

#define STOP_EVENT_NAME_BASE "{48c3c996-5cd9-44b5-9b27-03868db75de4}"

static uint StopEventHdl;

static void CloseStopEvent(void)
{
	LOGPOS();
	handleClose(StopEventHdl);
}
static void OpenStopEvent(void)
{
	char *eventName = xcout("%s.%u", STOP_EVENT_NAME_BASE, PortNo);

	LOGPOS();
	StopEventHdl = eventOpen(eventName);
	addFinalizer(CloseStopEvent);
	memFree(eventName);
}
static int StopEvent_HasSignal(void)
{
	if (collectEvents(StopEventHdl, 0))
	{
		LOGPOS();
		return 1;
	}
	return 0;
}
static void StopEvent_Set(void)
{
	LOGPOS();
	eventSet(StopEventHdl);
}

// ----

static int Idle(void)
{
	if (rbtGetCount(SessionTree))
	{
		uint remain;

		rbtSeekRoot(SessionTree);
		rbtJumpToLastAccess(SessionTree);

		for (remain = rbtGetCount(SessionTree); remain; remain--)
		{
			uint currKey = rbtGetCurrKey(SessionTree);
			Session_t *session = (Session_t *)rbtGetCurrValue(SessionTree);

			// action
			{
				IsCommDead(session->Comm);
			}

			rbtJumpForLeft(SessionTree);

			if (IsNoConnectTimeout(session)) // 削除
			{
				cout("Information: 無接続タイムアウト\n");
setConsoleColor(0x0d); // test
				rbtUnaddValue(SessionTree, currKey);
				ReleaseSession(session);
			}
		}
	}

	while (hasKey())
		if (getKey() == 0x1b)
			ProcDeadFlag = 1;

	if (StopEvent_HasSignal())
		ProcDeadFlag = 1;

	return !ProcDeadFlag;
}
int main(int argc, char **argv)
{
	PortNo = toValue(nextArg());
	FwdHost = nextArg();
	FwdPortNo = toValue(nextArg());

	errorCase(!m_isRange(PortNo, 1, 65535));
	errorCase(m_isEmpty(FwdHost));
	errorCase(!m_isRange(FwdPortNo, 1, 65535));

	OpenStopEvent();

readArgs:
	if (argIs("/S"))
	{
		StopEvent_Set();
		goto endFunc;
	}
	if (argIs("/C"))
	{
		ConnectMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/SS"))
	{
		SendSizeMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/RS"))
	{
		RecvSizeMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/T"))
	{
		SockTimeoutSec = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/NCT"))
	{
		NoConnectTimeoutSec = toValue(nextArg());
		goto readArgs;
	}
	errorCase_m(hasArgs(1), "不明なオプションが指定されました。");

	errorCase(!m_isRange(ConnectMax, 1, UINTMAX));
	errorCase(!m_isRange(SendSizeMax, HEADER_SIZE + 1, UINTMAX));
	errorCase(!m_isRange(RecvSizeMax, HEADER_SIZE + 1, UINTMAX));
	errorCase(!m_isRange(SockTimeoutSec, 1, 86400));
	errorCase(!m_isRange(NoConnectTimeoutSec, 1, 86400));

	cmdTitle_x(xcout("boomServer - %u %s:%u %u %u %u %u %u", PortNo, FwdHost, FwdPortNo, ConnectMax, SendSizeMax, RecvSizeMax, SockTimeoutSec, NoConnectTimeoutSec));

	SendSizeMax -= HEADER_SIZE; // to データサイズ

	SessionTree = rbCreateTree(ST_DuplKey, ST_CompKey, ST_ReleaseKey);

	SockStartup(); // sockServerUserTransmit() 終了後も接続が残るため

	critical(); // HACK: ここを ciritcal() ～ uncritical() で囲っている理由が分からない。不要ではないだろうか。@ 2020.8.xx
	{
		sockServerUserTransmit(Perform, CreateInfo, ReleaseInfo, PortNo, REAL_CONNECT_MAX, Idle);
	}
	uncritical();

	rbtCallAllValue(SessionTree, (void (*)(uint))ReleaseSession); // ここで全て切断する。

	SockCleanup();

	cmdTitle("boomServer");

endFunc:
	termination(0);
}
