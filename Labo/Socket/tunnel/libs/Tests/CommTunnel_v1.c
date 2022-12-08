/*
	CommTunnel.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... í‚é~Ç∑ÇÈÅB

	CommTunnel.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX]

		CONNECT-MAX ... ç≈ëÂê⁄ë±êî, è»ó™éûÇÕ 1000
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

	cout("[%u] ê⁄ë±\n", connectNo);

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
		endFlag = m_01(IsCommDead(a)) | m_01(IsCommDead(b)); // óºï˚é¿çsÇµÇΩÇ¢ÅB

		inner_uncritical();
		{
			sleep(waitMillis);
		}
		inner_critical();
	}

	ReleaseComm(a);
	ReleaseComm(b);

	cout("[%u] êÿíf\n", connectNo);
}
static int ReadArgs(void)
{
	return 0;
}
int main(int argc, char **argv)
{
	TunnelMain(ReadArgs, Perform, "CommTunnel", NULL);
}
