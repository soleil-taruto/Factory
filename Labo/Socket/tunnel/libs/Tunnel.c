#include "Tunnel.h"

#define STOPEVENTUUID "{f7749b48-ebaa-4fdb-93e0-14a1b8c7f29b}"

#define HOST_LENMAX 255
#define DEF_CONNECT_MAX 1000

uint PortNo;
char *FwdHost;
uint FwdPortNo;
uint ConnectMax = DEF_CONNECT_MAX;

static char *StopEventName;
static uint StopEventHdl;

static void (*UserPerform)(int sock, int fwdSock);

void (*TunnelKeyEvent)(int key) = (void (*)(int))noop_u;

static void PerformTh(int sock, char *strip)
{
	uchar ip[4] = { 0 };
	int fwdSock;

	fwdSock = sockConnect_NB(ip, FwdHost, FwdPortNo);

	if (fwdSock == -1)
		return;

	errorCase(!UserPerform);

	UserPerform(sock, fwdSock);
	sockDisconnect(fwdSock);
}
static int IdleTh(void)
{
	int clsFlag = 0;

	while (hasKey())
	{
		int key = getKey();

		if (key == 0x20)
			clsFlag = 1;

		if (key == 0x1b) // ? エスケープキー押下 -> 停止要求
			ProcDeadFlag = 1;

		if (key == '1')
			setDefConsoleColor();

		// この場所で '0' ～ '9' を予約

		// ENTER は使用しないこと！

		TunnelKeyEvent(key);
	}
	if (handleWaitForMillis(StopEventHdl, 0)) // ? 停止要求
		ProcDeadFlag = 1;

	if (clsFlag)
		execute("CLS");

	if (!ProcDeadFlag)
		return 1;

	cout("停止します...\n");
	return 0;
}

void (*TunnelPerformTh)(int sock, char *strip) = PerformTh;

/*
	userReadArgs      - NULL不可
	userPerform       - NULL不可 (但しTunnelPerformThを設定していればNULL可)
	title             - NULL不可
	cb_getTitleSuffix - NULL可
*/
void TunnelMain(int (*userReadArgs)(void), void (*userPerform)(int sock, int fwdSock), char *title, char *(*cb_getTitleSuffix)(void))
{
	errorCase(!userReadArgs);
	// userPerform
	errorCase(!title);
	// cb_getTitleSuffix

	PortNo = toValue(nextArg());
	FwdHost = nextArg();
	FwdPortNo = toValue(nextArg());

	errorCase(!PortNo || 0xffff < PortNo);
	errorCase(m_isEmpty(FwdHost) || HOST_LENMAX < strlen(FwdHost));
	errorCase(!FwdPortNo || 0xffff < FwdPortNo);

	StopEventName = xcout(STOPEVENTUUID ".%u", PortNo);
	StopEventHdl = eventOpen(StopEventName);

readArgs:
	if (argIs("/S"))
	{
		eventWakeupHandle(StopEventHdl);
		return;
	}
	if (argIs("/C"))
	{
		ConnectMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/DOSTO")) // HACK コメントに記載
	{
		DOSTimeoutSec = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/CCNDTO")) // HACK コメントに記載
	{
		CC_NoDataTimeoutSec = toValue(nextArg());
		goto readArgs;
	}
	if (userReadArgs())
	{
		goto readArgs;
	}
	errorCase_m(hasArgs(1), "不明なオプションが指定されました。");

	errorCase(!ConnectMax); // どのみち sockServerTh() でエラーになるけど..
	errorCase(IMAX < DOSTimeoutSec);

	cmdTitle_x(xcout("%s from localhost:%u to %s:%u C:%u %s (%u,%u)", title, PortNo, FwdHost, FwdPortNo, ConnectMax, cb_getTitleSuffix ? cb_getTitleSuffix() : "", DOSTimeoutSec, CC_NoDataTimeoutSec));

	UserPerform = userPerform;
	sockServerTh(TunnelPerformTh, PortNo, ConnectMax, IdleTh);
	UserPerform = NULL;

	handleClose(StopEventHdl);
}

// ---- Tools ----

static int IsTight_Main(void)
{
	if (10 <= sockConnectMax)
		return 0.8999 < sockConnectedRate;

	if (4 <= sockConnectMax)
		return sockConnectMax - 1 <= sockConnectedCount;

	if (2 <= sockConnectMax)
		return sockConnectMax <= sockConnectedCount;

	return 0;
}
int IsTight(void)
{
	if (IsTight_Main())
	{
		cout("TIGHT!\n");
		return 1;
	}
	return 0;
}

// ----
