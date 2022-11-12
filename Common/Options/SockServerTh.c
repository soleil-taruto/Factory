/*
	sockServer�̃}���`�X���b�h��, �ڑ������X���b�h�𐶐�����B

	----

	�ň��̃������g�p�� (*1)
		0 [byte] (*2)

	�ň��̃f�B�X�N�g�p�� (*1)
		0 [byte]

	(*1) �V�X�e�����E�R�[���o�b�N���ŏ���镪�A�X�^�b�N�ɐς܂�镪�ȂǍׂ������͍̂l�����Ă��Ȃ��B
	(*2) �ڑ����� (TransInfos, TransInfoStock, TransInfoDead �Ɋm�ۂ����u���b�N == 28 �o�C�g) ���g�p���邪�A
	     �ő�ڑ����� 65000 ���Ƃ��Ă� 1.82 MB ���x�Ȃ̂Ŗ�������B

	�R�[���o�b�N���l�������ň��̃������E�f�B�X�N�g�p��
		�R�[���o�b�N�Ƃ��� funcTransmit, funcIdle ������B
		���ꂼ��̌Ăяo���̂��߂ɑ�K�͂ɓW�J�����t�@�C���E�������u���b�N�͖������A���ꂼ��ʁX�̃X���b�h����Ăяo����A
		�ň��̏ꍇ (funcTransmit * �ő�ڑ��� + funcIdle * 1) �������Ɏ��s�����B(*3)
		�ň��̎g�p�ʂ� (funcTransmit �ł̍ő�g�p��) * connectmax + (funcIdle �ł̍ő�g�p��) �Ƃ��čl����B

	(*3) funcIdle ��(���ʂ�)�ʐM���s��Ȃ��� funcTransmit �� sockWait ���ɌĂяo�����̂Ń}���`�œ����B
*/

#include "SockServerTh.h"

#define MILLIS_TIMEOUT_SELECT_ACCEPT 1000
#define MILLIS_TIMEOUT_TRANSMIT_END 300

#define TRANSMIT_END_EVENT "cerulean.charlotte Factory SockServerTh transmit end event object"

typedef struct TransInfo_st
{
	int Sock;
	char ClientStrIp[16];
	uint Handle;
}
TransInfo_t;

static TransInfo_t *TransInfos;
static TransInfo_t **TransInfoStock;
static uint TransInfoStockCount;

static TransInfo_t **TransInfoDead;
static uint TransInfoDeadCount;

static void (*FuncTransmit)(int, char *);

static void PerformTransmit(uint prm)
{
	TransInfo_t *i = (TransInfo_t *)prm;

	critical();
	{
		FuncTransmit(i->Sock, i->ClientStrIp);

		SockPreClose(i->Sock);
		shutdown(i->Sock, SD_BOTH);
		closesocket(i->Sock);

		TransInfoDead[TransInfoDeadCount] = i;
		TransInfoDeadCount++;

		sockConnectedCount--;
		sockConnectMonUpdated();

		eventWakeup(TRANSMIT_END_EVENT);
	}
	uncritical();
}
/*
	funcTransmit(), funcIdle() �� critical(); �` uncritical(); �̒�����ĂԁB
*/
void sockServerTh(void (*funcTransmit)(int, char *), uint portno, uint connectmax, int (*funcIdle)(void))
{
	int tmpval;
	int sock;
	int clsock;
	struct sockaddr_in sa;
	struct sockaddr_in clsa;
	uint transmitEndEvent = eventOpen(TRANSMIT_END_EVENT);
	TransInfo_t *i;
	uint index;
	int shutdownTheServer = 0;

	errorCase(!funcTransmit);
	errorCase(!portno || 0xffff < portno);
	errorCase(!connectmax || UINTMAX / sizeof(TransInfo_t) < connectmax);
	errorCase(!funcIdle);

	SockStartup();

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	errorCase(sock == -1);
	SockPostOpen(sock);

	memset(&sa, 0x00, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons((unsigned short)portno);

	if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) != 0) // ? == -1
	{
		error_m("TCP�|�[�g�̃o�C���h�Ɏ��s���܂����B");
	}
	if (listen(sock, SOMAXCONN) != 0) // ? == -1
	{
		error();
	}

	TransInfos     = (TransInfo_t  *)memAlloc(connectmax * sizeof(TransInfo_t));
	TransInfoStock = (TransInfo_t **)memAlloc(connectmax * sizeof(TransInfo_t *));
	TransInfoDead  = (TransInfo_t **)memAlloc(connectmax * sizeof(TransInfo_t *));

	for (index = 0; index < connectmax; index++)
	{
		TransInfoStock[index] = TransInfos + index;
	}
	TransInfoStockCount = connectmax;
//	TransInfoDeadCount = 0;

	FuncTransmit = funcTransmit;

	sockConnectMax = connectmax;
	sockConnectMonUpdated();

	sockServerMode = 1;

	// ��������}���`�œ��� --->

	critical();
	{
		for (; ; )
		{
//			SockCurrTime = time(NULL);

			if (funcIdle() == 0) // ? �T�[�o�[��~
			{
				shutdownTheServer = 1;
			}
			else if (TransInfoStockCount)
			{
				int hasConnect;

				/*
					�����׎��ASockWait(, 1000, 0) �� 20 �b�҂�����邱�Ƃ�����B
					�ʃX���b�h�������Ă���̂Ńu���b�N����Ă��Ȃ��B-> select �ő҂�����Ă���Ǝv����B
					<- connect() �Ńu���b�N����� select �����CS�ɓ���Ȃ�����..
					20�b -> connect �̃^�C���A�E�g�H

					-> sockConnet() �� connect() ���u���b�N���Ă������������ł����B
				*/

#if 0
				hasConnect = SockWait(sock, MILLIS_TIMEOUT_SELECT_ACCEPT, 0);
#else
				{
					uint st;
					uint et;
					uint dt;

					st = now();

					hasConnect = SockWait(sock, MILLIS_TIMEOUT_SELECT_ACCEPT, 0);

					et = now();
					dt = et - st;

					if (2 < dt)
					{
						cout("Warning: �҂��󂯃|�[�g�̐ڑ��҂��� %u �b�|����܂����BhasConnect: %d\n", dt, hasConnect);
						cout("SockWait_T1: %u\n", SockWait_T1);
						cout("SockWait_T2: %u\n", SockWait_T2);
						cout("SockWait_T3: %u\n", SockWait_T3);
						cout("SockWait_T4: %u\n", SockWait_T4);
						setConsoleColor(0x5f);
					}
				}
#endif

				if (hasConnect) // ? �ڑ�����
				{
					tmpval = sizeof(clsa);
					clsock = accept(sock, (struct sockaddr *)&clsa, &tmpval);
					errorCase(clsock == -1);
					SockPostOpen(clsock);

					TransInfoStockCount--;
					i = TransInfoStock[TransInfoStockCount];

					i->Sock = clsock;
					strcpy(i->ClientStrIp, inet_ntoa(clsa.sin_addr));
					i->Handle = runThread(PerformTransmit, (uint)i);

					sockConnectedCount++;
					sockConnectMonUpdated();
				}
			}
			else
			{
			waitForTransmitEnd:
				inner_uncritical();
				{
					collectEvents(transmitEndEvent, MILLIS_TIMEOUT_TRANSMIT_END);
				}
				inner_critical();
			}
			collectEvents(transmitEndEvent, 0); // cleanup

			for (index = 0; index < TransInfoDeadCount; index++)
			{
				i = TransInfoDead[index];

				eventSleep(i->Handle); // i->Handle �̓X���b�h�����Ǐ����͓��������炱��ł�����...

				TransInfoStock[TransInfoStockCount] = i;
				TransInfoStockCount++;
			}
			TransInfoDeadCount = 0;

			if (shutdownTheServer)
			{
				if (TransInfoStockCount == connectmax)
					break;

				goto waitForTransmitEnd;
			}
		}
	}
	uncritical();

	// <--- �����܂Ń}���`�œ���

	SockPreClose(sock);

	if (closesocket(sock) != 0) // ? fault
	{
		error();
	}
	SockCleanup();

	memFree(TransInfos);
	memFree(TransInfoStock);
	memFree(TransInfoDead);
}
