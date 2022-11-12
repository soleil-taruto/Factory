#include "Comm.h"

#define BUFF_FULL 65000

static void Recv_Th(uint prm)
{
	Comm_t *i = (Comm_t *)prm;
	autoBlock_t *tmpBuff;
	int ret;
	uint waitMillis = 0;

	critical();
	{
		cout("受信ループ開始\n");

		while (!i->DeadFlag)
		{
			if (getSize(i->RecvBuff) < BUFF_FULL)
			{
				/*
					i->RecvBuff は受信待機中に変更される可能性があるので SockRecvSequ() には渡さない。
				*/
				tmpBuff = newBlock();
				ret = SockRecvSequ(i->Sock, tmpBuff, waitMillis);
				ab_addBytes_x(i->RecvBuff, tmpBuff);

				if (ret == -1)
				{
					cout("RecvLoopDead: %u, LastByte: %02x\n", getSize(i->RecvBuff), getSize(i->RecvBuff) ? getByte(i->RecvBuff, getSize(i->RecvBuff) - 1) : 0x00);

					i->SockRecvClosedFlag = 1;
					i->SockClosedFlag = 1;
					i->DeadFlag = 1;
					break;
				}
				if (ret)
					waitMillis = 0;
				else
					waitMillis = m_min(waitMillis + 1, 2000);
			}
			else
			{
				inner_uncritical();
				{
					IntSleep(2000);
				}
				inner_critical();

				waitMillis = 0;
			}
		}
		cout("受信ループ終了\n");
	}
	uncritical();
}
static void Send_Th(uint prm)
{
	Comm_t *i = (Comm_t *)prm;
	autoBlock_t *tmpBuff;
	int ret;
	uint waitMillis = 2000;
	uint abortTime = UINTMAX;

	critical();
	{
		cout("送信ループ開始\n");

		while (!i->SockClosedFlag)
		{
			if (getSize(i->SendBuff))
			{
				/*
					i->SendBuff は送信待機中に変更される可能性があるので SockSendSequ() には渡さない。
				*/
				tmpBuff = i->SendBuff;
				i->SendBuff = newBlock();
				ret = SockSendSequ(i->Sock, tmpBuff, waitMillis);
				ab_addBytes_x(tmpBuff, i->SendBuff);
				i->SendBuff = tmpBuff;

				if (i->DeadFlag)
					cout("DeadFlaggedSend_ret: %d (%u, %u)\n", ret, getSize(i->SendBuff), waitMillis);

				if (ret == -1)
				{
					LOGPOS();

					i->SockClosedFlag = 1;
					i->DeadFlag = 1;
					break;
				}
				if (i->DeadFlag)
				{
					if (i->KillFlag)
						break;

					if (abortTime == UINTMAX)
						abortTime = DOSTimeoutSec ? now() + DOSTimeoutSec : UINTMAX - 1;

					if (now() < abortTime)
					{
						cout("★★★送信タイムアウト_Comm_size: %u\n", getSize(i->SendBuff));
						break;
					}
					cout("☆☆☆Comm_T %u\n", getSize(i->SendBuff));

					if (ret)
						waitMillis = 0;
					else
						waitMillis = 100;

					break;
				}
			}
			else
			{
				if (i->DeadFlag)
					break;

				inner_uncritical();
				{
					IntSleep(2000);
				}
				inner_critical();
			}
		}
		cout("送信ループ終了\n");
	}
	uncritical();
}

Comm_t *CreateComm(int sock)
{
	Comm_t *i = (Comm_t *)memAlloc(sizeof(Comm_t));

	i->Sock = sock;
	i->SockClosedFlag = 0;
	i->SockRecvClosedFlag = 0;
	i->RecvBuff = newBlock();
	i->SendBuff = newBlock();
	i->DeadFlag = 0;
	i->KillFlag = 0;
	i->ThreadEndedFlag = 0;
	i->RecvTh = runThread(Recv_Th, (uint)i);
	i->SendTh = runThread(Send_Th, (uint)i);

	return i;
}
void EndCommThread(Comm_t *i)
{
	if (i->ThreadEndedFlag)
		return;

	i->DeadFlag = 1;

	inner_uncritical();
	{
		IntSleepInt();

		waitThread(i->RecvTh);
		waitThread(i->SendTh);
	}
	inner_critical();

	i->ThreadEndedFlag = 1;
}
void ReleaseComm(Comm_t *i)
{
	i->KillFlag = 1;

	EndCommThread(i);
	releaseAutoBlock(i->RecvBuff);
	releaseAutoBlock(i->SendBuff);
	memFree(i);
}

autoBlock_t *GetCommRecvData(Comm_t *i, uint size)
{
	autoBlock_t *data;

	if (IsCommDead(i) && !i->SockRecvClosedFlag && getSize(i->RecvBuff) < BUFF_FULL)
	{
		int ret = SockRecvSequ(i->Sock, i->RecvBuff, 0);

		cout("スレッド終了後の受信_ret: %d\n", ret);

		if (ret <= 0)
			i->SockRecvClosedFlag = 1;
	}

	data = desertBytes(i->RecvBuff, 0, m_min(size, getSize(i->RecvBuff)));

	if (getSize(data))
		IntSleepInt();

	return data;
}
int AddCommSendData(Comm_t *i, autoBlock_t *data, int forceMode) // ret: ? 追加した。
{
	int retval = 0;

	if (IsCommDead(i))
	{
		LOGPOS();
		return 1;
	}

	if (getSize(i->SendBuff) < BUFF_FULL || forceMode)
	{
		addBytes(i->SendBuff, data);
		retval = 1;
	}
	if (getSize(i->SendBuff))
		IntSleepInt();

	return retval;
}
int IsCommDead(Comm_t *i)
{
	if (i->DeadFlag)
	{
		EndCommThread(i);
		return 1;
	}
	return 0;
}
int IsCommDeadAndEmpty(Comm_t *i)
{
	return IsCommDead(i) && i->SockRecvClosedFlag && !getSize(i->RecvBuff);
}
