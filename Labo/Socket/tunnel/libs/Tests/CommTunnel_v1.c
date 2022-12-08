/*
	CommTunnel.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... ��~����B

	CommTunnel.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX]

		CONNECT-MAX ... �ő�ڑ���, �ȗ����� 1000
*/

#include "..\Comm.h"
#include "..\Tunnel.h"

static uint ConnectNoCounter;

static void Perform(int sock, int fwdSock)
{
	Comm_t *a;
	Comm_t *b;
	uint waitMillis = 0;
	int endFlag = 0;
	uint connectNo = ConnectNoCounter;

	if (ConnectNoCounter == UINTMAX)
		ConnectNoCounter = 0;
	else
		ConnectNoCounter++;

	cout("[%u] �ڑ�\n", connectNo);

	a = CreateComm(sock);
	b = CreateComm(fwdSock);

	while (!ProcDeadFlag)
	{
		autoBlock_t *aToB = GetCommRecvData(a, UINTMAX);
		autoBlock_t *bToA = GetCommRecvData(b, UINTMAX);
		uint aToBSize;
		uint bToASize;

		aToBSize = getSize(aToB);
		bToASize = getSize(bToA);

		AddCommSendData(b, aToB, 1);
		AddCommSendData(a, bToA, 1);

		releaseAutoBlock(aToB);
		releaseAutoBlock(bToA);

		cout("[%u] %u %u (%d %u)\n", connectNo, aToBSize, bToASize, endFlag ? 1 : 0, waitMillis);

		if (!aToBSize && !bToASize)
		{
			if (endFlag)
				break;

			waitMillis = m_min(waitMillis + 1, 2000);
		}
		else
		{
			waitMillis = 0;
		}
		endFlag = m_01(IsCommDead(a)) | m_01(IsCommDead(b)); // �������s�������B

		inner_uncritical();
		{
			sleep(waitMillis);
		}
		inner_critical();
	}

	ReleaseComm(a);
	ReleaseComm(b);

	cout("[%u] �ؒf\n", connectNo);
}
static int ReadArgs(void)
{
	return 0;
}
int main(int argc, char **argv)
{
	TunnelMain(ReadArgs, Perform, "CommTunnel", NULL);
}
