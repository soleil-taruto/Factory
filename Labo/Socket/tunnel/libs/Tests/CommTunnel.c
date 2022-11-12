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
	uint connectNo = ConnectNoCounter;
	uint stress = 0;

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

		AddCommSendData(b, aToB, 1);
		AddCommSendData(a, bToA, 1);

		if (getSize(aToB) || getSize(bToA))
			waitMillis = 0;
		else
			waitMillis = m_min(waitMillis + 1, 2000);

		cout("[%u] %u %u (%u)\n", connectNo, getSize(aToB), getSize(bToA), waitMillis);

		releaseAutoBlock(aToB);
		releaseAutoBlock(bToA);

		if (m_01(IsCommDeadAndEmpty(a)) & m_01(IsCommDeadAndEmpty(b))) // �������s�������B
			break;

		// TODO
		// ������΍�
		// ���̖��Ɋ֌W���邩�� --> C:\Dev\wb\t20200122_GBCTunnelTest\memo.txt
		{
			stress += m_01(IsCommDeadAndEmpty(a)) | m_01(IsCommDeadAndEmpty(b));

			if (100 < stress) // �������l�͓K���B�������K�v
				break;
		}

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
