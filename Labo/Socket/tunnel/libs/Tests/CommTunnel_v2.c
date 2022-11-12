/*
	CommTunnel.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	CommTunnel.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX]

		CONNECT-MAX ... 最大接続数, 省略時は 1000
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

	cout("[%u] 接続\n", connectNo);

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

		if (m_01(IsCommDead(a)) | m_01(IsCommDead(b))) // 両方実行したい。
			break;

		inner_uncritical();
		{
			sleep(waitMillis);
		}
		inner_critical();
	}

	while (!ProcDeadFlag)
	{
		autoBlock_t *aToB = GetCommRecvData(a, UINTMAX);
		autoBlock_t *bToA = GetCommRecvData(b, UINTMAX);
		int endFlag;

		AddCommSendData(b, aToB, 1);
		AddCommSendData(a, bToA, 1);

		endFlag = !getSize(aToB) && !getSize(bToA);

		releaseAutoBlock(aToB);
		releaseAutoBlock(bToA);

		if (endFlag)
			break;

		if (m_01(IsCommDead(a)) & m_01(IsCommDead(b))) // 両方実行したい。
			break;
	}

	ReleaseComm(a);
	ReleaseComm(b);

	cout("[%u] 切断\n", connectNo);
}
static int ReadArgs(void)
{
	return 0;
}
int main(int argc, char **argv)
{
	TunnelMain(ReadArgs, Perform, "CommTunnel", NULL);
}
