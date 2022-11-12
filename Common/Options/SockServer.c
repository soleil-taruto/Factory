/*
	C:\Factory\Common\all.h ����� SockServer.h ���C���N���[�h���邩 all.h �̃C���N���[�h���ȗ����邱�ƁB

	�Ăяo���V�[�P���X(�P�v���Z�X):
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

	�ň��̃������g�p�� (*1)
		sockServer()
			�ő�ڑ��� * (SockFile_t �o�b�t�@�T�C�Y @ SockRecvFile, SockSendFile) ==
			connectmax * (1024 * 1024) [byte]

		sockServerUserTransmit()
			0 [byte]

	�ň��̃f�B�X�N�g�p�� (*1)
		sockServer()
			�ő�ڑ��� * (��M�f�[�^�ő�T�C�Y + (�����f�[�^�ő�T�C�Y == funcPerform() �̏o�͂���t�@�C���̍ő�T�C�Y)) ==
			connectmax * (uploadmax + �����f�[�^�ő�T�C�Y)

		sockServerUserTransmit()
			0 [byte]

	(*1) �V�X�e�����E�R�[���o�b�N���ŏ���镪�A�X�^�b�N�ɐς܂�镪�ȂǍׂ������͍̂l�����Ă��Ȃ��B

	�R�[���o�b�N���l�������ň��̃������E�f�B�X�N�g�p��
		�R�[���o�b�N�Ƃ��� funcPerform, funcTransmit, funcCreateUserInfo, funcReleaseUserInfo, funcIdle ������B
		���ꂼ��̌Ăяo���̂��߂ɑ�K�͂ɓW�J�����t�@�C���E�������u���b�N�͖����A�S�ăA�g�~�b�N�Ɏ��s�����̂ŁA
		�P���Ɂu�ň��̃������E�f�B�X�N�g�p�ʁv�ɃR�[���o�b�N�ł̍ň��̎g�p�ʂ����������̂Ƃ���B
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

		t->Death = memcmp(t->Signature->Block, SOCK_SIGNATURE, t->Signature->BlockSize); // ? �s���ȃV�O�l�`��

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
			cout("| ��M�f�[�^�t�@�C���T�C�Y�������𒴂��Ă��܂� |\n");
			cout("| memo: uploadmax ������������H               |\n");
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

		if (InterruptMode) // funcPerform() �ċA�Ăяo���h�~
			return;

		*(uint *)sockClientIp = *(uint *)t->ClientIp;

		if (FuncPerform(t->PrmFile->File, t->AnsFile->File) == 0) // ? �G���[�܂��͎��s
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
		t->Death = SockTransmitBlock(t->Sock, t->AnsAck, waitMillis, 0) == -1; // �����Ŏ���ł�����

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
				continue; // �����֓��B����P�[�X�F�ؒf�������Ă��� sockServerPerformInterrupt() ����ċA�I�ɌĂяo���ꂽ�B

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
	sockServer(), sockServerEx() �ɂ����� funcPerform() �̂P��̎��s�͐��b�ŏI��邱�Ƃ�z�肵�Ă���B
	�����|�������肷��Ƒ��̐ڑ��� SockTransmitBlock() �̃^�C���A�E�g�Őؒf����Ă��܂����낤�B
	���������邽�߁A�����Ԋ|���� funcPerform() �̎��s���͒���I�� sockServerPerformInterrupt() ���ĂԂ��ƁB
*/
void sockServerPerformInterrupt(void)
{
	uint index = findElement(Transmissions, 0, simpleComp);

	if (index < SSPIConnectMax)
		if (SockWait(SSPISock, 0, 0)) // ? �ڑ�����
			putElement(Transmissions, index, (uint)CreateTransmissionEx(SSPISock));

	InterruptMode = 1; // PerformTransmit() ����ċA�I�� funcPerform ���Ăяo���Ȃ��悤��
	Transmission();
	InterruptMode = 0;
}

/*
	int funcPerform(char *prmFile, char *ansFile);
		RPC�ۂ��ʐM���s���B
		sockClientIp ���L��

		prmFile
			�v���f�[�^���L�q�����o�C�i���t�@�C���A�ύX���Ă��悢���A�폜���Ă͂Ȃ�Ȃ��B(�{�֐��̏I�����ɑ��݂���Ηǂ�)

		ansFile
			�����f�[�^���L�q����o�C�i���t�@�C��
			�{�֐����J�n�������_�ł� 0 �o�C�g�̃t�@�C�������݂���B
			�{�֐����I������O�ɁA�����f�[�^�������o�����ƁB(�G���[�̏ꍇ�ł�) �폜���Ă͂Ȃ�Ȃ��B(�{�֐��̏I�����ɑ��݂���Ηǂ�)
			�����f�[�^�� 0 �` UINT64MAX �o�C�g��z�肵�Ă������B

		�߂�l
			0 �ȊO == ����I��
			0      == �G���[�܂��͎��s�A�����ɐڑ���ؒf����B

		prmFile, ansFile ���Ɍ�茟�o�E�������s��Ȃ��B-> PadFile.c ���g���悤��..

	void funcTransmit(int sock, uint userInfo);
		�Ăяo�����̕��@�ŒʐM���s���B
		funcTransmit(), funcCreateUserInfo(), funcReleaseUserInfo() ����̌Ăяo������ sockClientIp ���L��

		sock
			�ڑ����̃\�P�b�g�AfuncTransmit() ���� shutdown(), closesocket() ���Ă͂Ȃ�Ȃ��B

		userInfo
			uint funcCreateUserInfo(void); �̖߂�l
			�ؒf���� void funcReleaseUserInfo(uint userInfo); �ɓn���B

		�߂�l
			0 �ȊO == ����I��
			0      == �G���[�܂��͒ʐM�I���A�����ɐڑ���ؒf����B

	portno - �|�[�g�ԍ� (1�`65535)
	connectmax - �ő哯���ڑ����A�������� 10�`20 ���炢�ɂ��Ă����������ǂ��񂶂�Ȃ����낤��
	uploadmax - �v���f�[�^�̍ő�T�C�Y

	�ň��̃������g�p��: connectmax * 1024^2 [byte] (*1)
	�ň��̃f�B�X�N�g�p��: connectmax * (uploadmax + �����f�[�^�ő�T�C�Y) [byte] (*1)

	(*1) �Ǘ��̈�Ƃ��ׂ����Ƃ���͍l�����Ȃ��B

	int funcIdle(void);
		�s����ɉ��x���Ăяo���B

		�߂�l
			0 �ȊO == �T�[�o�[�𑱍s����B
			0      == �T�[�o�[���~����B
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

		if (funcIdle() == 0) // ? �T�[�o�[��~
			break;

		Transmission();
		removeZero(Transmissions);

		if (getCount(Transmissions) < connectmax)
		{
			if (SockWait(sock, getCount(Transmissions) ? 0 : MILLIS_TIMEOUT_SELECT_ACCEPT, 0)) // ? �ڑ�����
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
