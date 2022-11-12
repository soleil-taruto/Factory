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

		if (key == 0x1b) // ? �G�X�P�[�v�L�[���� -> ��~�v��
			ProcDeadFlag = 1;

		if (key == '1')
			setDefConsoleColor();

		// ���̏ꏊ�� '0' �` '9' ��\��

		// ENTER �͎g�p���Ȃ����ƁI

		TunnelKeyEvent(key);
	}
	if (handleWaitForMillis(StopEventHdl, 0)) // ? ��~�v��
		ProcDeadFlag = 1;

	if (clsFlag)
		execute("CLS");

	if (!ProcDeadFlag)
		return 1;

	cout("��~���܂�...\n");
	return 0;
}

void (*TunnelPerformTh)(int sock, char *strip) = PerformTh;

/*
	userReadArgs      - NULL�s��
	userPerform       - NULL�s�� (�A��TunnelPerformTh��ݒ肵�Ă����NULL��)
	title             - NULL�s��
	cb_getTitleSuffix - NULL��
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
	if (argIs("/DOSTO")) // HACK �R�����g�ɋL��
	{
		DOSTimeoutSec = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/CCNDTO")) // HACK �R�����g�ɋL��
	{
		CC_NoDataTimeoutSec = toValue(nextArg());
		goto readArgs;
	}
	if (userReadArgs())
	{
		goto readArgs;
	}
	errorCase_m(hasArgs(1), "�s���ȃI�v�V�������w�肳��܂����B");

	errorCase(!ConnectMax); // �ǂ݂̂� sockServerTh() �ŃG���[�ɂȂ邯��..
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
