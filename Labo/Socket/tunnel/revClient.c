/*
	revClient.exe REV-HOST REV-PORT FWD-HOST FWD-PORT /S

		/S ... ��~����

	revClient.exe REV-HOST REV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/P POLLING-CYCLE]

		REV-HOST ... �t�����z�X�g��
		REV-PORT ... �t�����|�[�g�ԍ�
		FWD-HOST ... �]����z�X�g��
		FWD-PORT ... �]����|�[�g�ԍ�
		CONNECT-MAX ... �ő�ڑ���, �ȗ����� 1000
		POLLING-CYCLE ... �t�����ɖ₢���킹����Ԋu, �P�ʂ̓~���b, �ȗ����� 2000
*/

#include "libs\CrossChannel.h"
#include "C:\Factory\OpenSource\md5.h"

#define STOPEVENTUUID "{9aef9f06-2d21-4387-abe5-c3c079dfb1d2}"

#define DEF_CONNECT_MAX 1000
#define DEF_POLLING_MILLIS 2000
#define RECV_C_TIMEOUT_MILLIS 60000
#define RECV_C_ONCE_MILLIS 2000

static char *RevHost;
static uint RevPortNo;
static char *FwdHost;
static uint FwdPortNo;
static uint ConnectMax = DEF_CONNECT_MAX;
static uint PollingMillis = DEF_POLLING_MILLIS;

static char *StopEventName;
static uint StopEventHdl;

static uint ConnectCount;

static void TransmitTh(int sockPair[2])
{
	int sock = sockPair[0];
	int fwdSock = sockPair[1];

	critical();
	{
		CrossChannel(sock, fwdSock, NULL, 0, NULL, 0);

		ConnectCount--;
		cout("�ؒf(%u) %d -> %d\n", ConnectCount, sock, fwdSock);

		sockDisconnect(sock);
		sockDisconnect(fwdSock);
	}
	uncritical();
}
static void PM_Interval(void)
{
	while (hasKey())
		if (getKey() == 0x1b) // ? �G�X�P�[�v�L�[���� -> ��~�v��
			ProcDeadFlag = 1;

	if (handleWaitForMillis(StopEventHdl, 0)) // ? ��~�v��
		ProcDeadFlag = 1;
}
static void PollingMain(void)
{
	critical();
	{
		for (; ; )
		{
			int sock;

			PM_Interval();

			if (ProcDeadFlag)
				break;

LOGPOS(); // test
			if (ConnectCount < ConnectMax)
				sock = sockConnect(NULL, RevHost, RevPortNo);
			else
				sock = -1;

			if (sock != -1) // ? ����
			{
				int connectFlag;

				{
					autoBlock_t *buff = newBlock();
					int retval;
					int retcode;

LOGPOS(); // test
#if 1
					{
						uint millis = 0;

						do
						{
							retval = SockRecvSequLoop(sock, buff, RECV_C_ONCE_MILLIS, 1);

							if (retval != 0)
								break;

							PM_Interval();

							if (ProcDeadFlag)
								break;

							millis += RECV_C_ONCE_MILLIS;
						}
						while (millis < RECV_C_TIMEOUT_MILLIS);
					}
#else // old
					retval = SockRecvSequLoop(sock, buff, RECV_C_TIMEOUT_MILLIS, 1);
#endif
LOGPOS(); // test
					retcode = refByte(buff, 0);
					releaseAutoBlock(buff);

					connectFlag = retval == 1 && retcode == 'C'; // ? �ڑ��A��
				}

				if (connectFlag)
				{
					int fwdSock = sockConnect(NULL, FwdHost, FwdPortNo);
LOGPOS(); // test

					if (fwdSock != -1)
					{
						int sockPair[2];

						ConnectCount++;
						cout("�ڑ�(%u) %d -> %d\n", ConnectCount, sock, fwdSock);

						sockPair[0] = sock;
						sockPair[1] = fwdSock;

						runThread((void (*)(uint))TransmitTh, (uint)sockPair);
LOGPOS(); // test

						continue; // sock �̐ؒf, �X���[�v��������΂��B
					}
					cout("�]����ɐڑ��ł��܂���ł����B\n");
				}
				sockDisconnect(sock);
			}
			if (ProcDeadFlag)
				break;

LOGPOS(); // test
			inner_uncritical();
			{
				sleep(PollingMillis);
			}
			inner_critical();
LOGPOS(); // test
		}
		cout("�I�����܂�...\n");

		// �S�ʐM�I���҂�
		{
			uint waitMillis = 0;

			while (ConnectCount)
			{
				inner_uncritical();
				{
					sleep(waitMillis);
				}
				inner_critical();

				if (waitMillis < 2000)
					waitMillis++;
			}
		}
	}
	uncritical();
}
int main(int argc, char **argv)
{
	RevHost = nextArg();
	RevPortNo = toValue(nextArg());
	FwdHost = nextArg();
	FwdPortNo = toValue(nextArg());

readArgs:
	if (argIs("/C"))
	{
		ConnectMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/P"))
	{
		PollingMillis = toValue(nextArg());
		goto readArgs;
	}

	errorCase(m_isEmpty(RevHost));
	errorCase(!RevPortNo || 0xffff < RevPortNo);
	errorCase(m_isEmpty(FwdHost));
	errorCase(!FwdPortNo || 0xffff < FwdPortNo);
	errorCase(!ConnectMax);
	// PollingMillis

	StopEventName = xcout(STOPEVENTUUID ".%s", c_md5_makeHexHashLine_x(xcout("%s.%u.%s.%u", RevHost, RevPortNo, FwdHost, FwdPortNo)));
	StopEventHdl = eventOpen(StopEventName);

	if (argIs("/S"))
	{
		eventWakeupHandle(StopEventHdl);
		return;
	}
	errorCase(hasArgs(1)); // �s���ȃI�v�V����

	cmdTitle_x(xcout("revClient from %s:%u to %s:%u C:%u P:%u", RevHost, RevPortNo, FwdHost, FwdPortNo, ConnectMax, PollingMillis));

	SockStartup();
	PollingMain();
	SockCleanup();
}
