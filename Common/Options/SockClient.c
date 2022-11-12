/*
	C:\Factory\Common\all.h ����� SockClient.h ���C���N���[�h���邩 all.h �̃C���N���[�h���ȗ����邱�ƁB

	�Ăяo���V�[�P���X(�P�v���Z�X):
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

	������ ����� SClient() ���g���悤�ɂ��ĂˁB
*/

#include "SockClient.h"

#define MILLIS_TIMEOUT_TRANSMIT 300
#define SEC_IP_TIMEOUT 180

/*
	(�����)�t�@�C�A�E�H�[����L���ɂ��Ă���� gethostbyname() �� IP �����Ȃ��B
	�u�t�@�C���ƃv�����^�̋��L�v���O�ɂ���Ǝ擾�ł���悤�ɂȂ�B
	ping �ɉ������邩���Ȃ������֌W�H
*/
autoList_t *sockLookupList(char *domain)
{
	autoList_t *ips = newList();
	struct hostent *host;
	struct in_addr addr;
	uint index;
	uint ip;

	SockStartup();
	host = gethostbyname(domain); // domain == "" -> ������IP�B������NIC��}���Ă���ƁA�����Ԃ�B

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
	ip, domain: NULL �s��
	domain �ɑ΂���ŏ���IP�A�h���X�� ip �Ɋi�[����B
	������Ȃ��Ƃ��� 0.0.0.0 ���i�[����B
	domain == "" -> ������IP
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

	// WSA_WAIT_TIMEOUT        <---- �^�C���A�E�g
	// or
	// WSA_WAIT_IO_COMPLETION  <---- This return value can only be returned if the fAlertable(��5����) parameter is TRUE.
	// or
	// WSA_WAIT_EVENT_0 �` (WSA_WAIT_EVENT_0 + cEvents - 1)

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
	�ȒP�Ȏg�p��
		SockStartup();
		sock = sockConnect(ip, domain, portno);
		if (sock != -1) {
			�����ŒʐM
			sockDisconnect(sock);
		}
		SockCleanup();
*/
int sockConnectEx(uchar ip[4], char *domain, uint portno, int nonBlocking) // ret: -1 == ���s, -1 �ȊO == ����, �����̂Ƃ��� sockDisconnect() ��Y�ꂸ��
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

			cout("%s: �A�N�e�B�u�I�[�v�����^�C���A�E�g���܂����B%u[ms] %s %s %d\n",
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

	if (retval == -1) // ? �ڑ����s
	{
		if (domain) // ? domain �L��
			*(uint *)ip = 0; // domain �ɑΉ����� ip ���X�V���ꂽ��������Ȃ��B-> ���� ip �X�V�̂��� 0.0.0.0 �Đݒ�

		SockPreClose(sock);
		closesocket(sock);
		return -1;
	}
	if (IsTimeout(ip))
	{
		LOGPOS(); // test
		*(uint *)ip = 0; // ����I�Ƀ��Z�b�g����B
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
	ip - �ڑ����IP�A�h���X�Adomain �ɐڑ�����ɂ� 0.0.0.0 ��ݒ肵�Ă����B
		sockLookup() �ɂ���čX�V�����B
		�ڑ��ł��Ȃ��Ƃ��Adomain ���L��(��NULL)�Ȃ�� 0.0.0.0 �ɍĐݒ肳���B
	domain - �ڑ���̃h���C�����A���ɖ����ꍇ�� NULL ���w�肷��B
	portno - �ڑ���̃|�[�g�ԍ� (1 �` 65535)
	prmFile - �v���f�[�^�A0 �` UINT64MAX �o�C�g�̑��݂���t�@�C���ł��邱�ƁB

	int idleFunc(void);
		�s����ɉ��񂩌Ăяo���B

		�߂�l
			0 �ȊO == ����I��
			0      == �ʐM�𒆎~���āAsockClientEx() �� NULL ��Ԃ��B

	void transmitFunc(int sock);
		�Ăяo�����̕��@�ŒʐM���s���B

		sock
			�ڑ����̃\�P�b�g�AtransmitFunc() ���� shutdown(), closesocket() ���Ă͂Ȃ�Ȃ��B

	�߂�l
		NULL �ȊO == ����I���A�����f�[�^���L�q�����o�C�i���t�@�C��
		             �t�@�C���̍폜�ƃt�@�C�����̊J���͌Ăяo�����̐ӔC
		NULL      == �ʐM�G���[

	prmFile, ansFile(�߂�l) ���Ɍ�茟�o�E�������s��Ȃ��B-> PadFile.c
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

	// �ʐM����
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
	����: �ڑ��Ɏ��s����� transmitFunc() �͎��s���ꂸ�ɏI������B
*/
void sockClientUserTransmit(uchar ip[4], char *domain, uint portno, void (*transmitFunc)(int))
{
	if (sockClientEx(ip, domain, portno, NULL, NULL, transmitFunc) != NULL)
	{
		error();
	}
}
