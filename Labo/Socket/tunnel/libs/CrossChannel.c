#include "CrossChannel.h"

typedef struct Channel_st
{
	int RSock;
	int WSock;
	void (*DataFltr)(autoBlock_t *, uint);
	uint FltrData;
	int *P_DeadFlag;
	uint *P_NoDataTimeoutTime;
}
Channel_t;

int ProcDeadFlag;
int ChannelDeadFlag; // tlsではないので、DataFltr実行中にスレッドが切り替わった先で変更されるかもしれない事に注意！

static void ChannelTransmit(uint prm)
{
	Channel_t *i = (Channel_t *)prm;

	critical();
	{
		autoBlock_t *rBuff = newBlock();
		autoBlock_t *wBuff = newBlock();
		uint rTmout = 0;
		int retval;
		uint lastDataFltrExecTime = now();
		int laterBreakFlag = 0;
		int dosFlag = 0;
		uint dosTmout;
		char *tmp;

		i->DataFltr(wBuff, i->FltrData);

		for (; ; )
		{
			int ioOccurred = 0;

			if (ProcDeadFlag)
				goto endTransmit;

			if (getSize(wBuff) < 1000000) // ? ! 送信バッファ溜まり過ぎ
			{
				int execDataFltrFlag = 0;

				if (getSize(wBuff))
					rTmout = 0;

				/*
					片方のソケットが切断された時、もう片方への SockRecvSequ() 内の select() は即座に終われない。
					-> 待ち時間を調整するようにした。
					切断された側への select() も何故か即座に終わらないように見えた。
					切断の検知に select() は問題ありなのかもしれない。
				*/
				retval = SockRecvSequ(i->RSock, rBuff, rTmout);

				if (retval == -1)
				{
					*i->P_DeadFlag = 1;
					break;
				}

				if (getSize(rBuff))
				{
					rTmout = 0;
					execDataFltrFlag = 1;
					ioOccurred = 1;
				}
				else
				{
					rTmout = m_min(rTmout + 1, 2000);
					execDataFltrFlag = lastDataFltrExecTime + 2 < now();
				}

				if (execDataFltrFlag)
				{
					ChannelDeadFlag = 0;
					i->DataFltr(rBuff, i->FltrData);

					if (ChannelDeadFlag)
					{
						*i->P_DeadFlag = 1;
						break;
					}
					lastDataFltrExecTime = now();
				}
				addBytes(wBuff, rBuff);
				setSize(rBuff, 0);
			}
			if (ProcDeadFlag)
				goto endTransmit;

			/*
				逆方向の SockSendSequ() が -1 を返した(i->RSock が切断した)場合でも i->RSock に読み込み可能なデータがあるかもしれない？
				あるとすれば i->WSock は生きてるかもしれないので、読み込めなくなるまで SockRecvSequ() を実行したい。
				-> i->P_DeadFlag の立ち上がりで即終了しないようにした。
			*/
			if (*i->P_DeadFlag) // ? 逆方向で切断を検知した。
			{
				if (dosFlag)
				{
					if (getSize(wBuff) == 0) // ? 受信データ・送信データ共に無いので終了
						goto endTransmit;

					if (dosTmout < now()) // ? タイムアウト
					{
						cout("★★★逆方向の切断を検知したのにタイムアウト_wBuff_size: %u @ %s\n", getSize(wBuff), tmp = makeJStamp(NULL, 0));
						memFree(tmp);
						goto endTransmit;
					}
					cout("☆☆☆CC_T_R %u @ %s\n", getSize(wBuff), tmp = makeJStamp(NULL, 0));
					memFree(tmp);
				}
				else
				{
					dosFlag = 1;
					dosTmout = GetTimeoutTime(DOSTimeoutSec);
				}
				rTmout = 0;
			}

			if (getSize(wBuff))
			{
				retval = SockSendSequ(i->WSock, wBuff, 2000);

				if (retval == -1)
				{
					*i->P_DeadFlag = 1;
					goto endTransmit;
				}

				if (retval)
				{
					ioOccurred = 1;
				}
			}

			if (ioOccurred)
			{
				*i->P_NoDataTimeoutTime = GetTimeoutTime(CC_NoDataTimeoutSec);
			}
			else if (*i->P_NoDataTimeoutTime < now())
			{
				cout("★★★CC無通信タイムアウト★★★\n");
				*i->P_DeadFlag = 1;
				goto endTransmit;
			}
		}

		{
			uint abortTime = GetTimeoutTime(DOSTimeoutSec);

			for (; ; )
			{
				if (ProcDeadFlag)
					break;

				retval = SockSendSequ(i->WSock, wBuff, 2000);

				if (retval == -1)
					break;

				if (getSize(wBuff) == 0)
					break;

				if (abortTime < now()) // ? タイムアウト
				{
					cout("★★★書き込みタイムアウト_wBuff_size: %u @ %s\n", getSize(wBuff), tmp = makeJStamp(NULL, 0));
					memFree(tmp);
					break;
				}
				cout("☆☆☆CC_T_S %u @ %s\n", getSize(wBuff), tmp = makeJStamp(NULL, 0));
				memFree(tmp);
			}
		}

	endTransmit:
		releaseAutoBlock(rBuff);
		releaseAutoBlock(wBuff);
	}
	uncritical();

	*i->P_DeadFlag = 1; // 2bs
}

void CrossChannel(
	int sockA,
	int sockB,
	void (*aToBFltr)(autoBlock_t *, uint),
	uint aToBFltrData,
	void (*bToAFltr)(autoBlock_t *, uint),
	uint bToAFltrData
	)
{
	Channel_t aToB;
	Channel_t bToA;
	uint aToBTh;
	uint bToATh;
	int deadFlag = 0;
	uint noDatTmoutTime = GetTimeoutTime(CC_NoDataTimeoutSec);

	errorCase(sockA == -1);
	errorCase(sockB == -1);
	// aToBFltr
	// aToBFltrData
	// bToAFltr
	// bToAFltrData

	if (!aToBFltr)
		aToBFltr = (void (*)(autoBlock_t *, uint))noop_uu;

	if (!bToAFltr)
		bToAFltr = (void (*)(autoBlock_t *, uint))noop_uu;

	aToB.RSock = sockA;
	aToB.WSock = sockB;
	aToB.DataFltr = aToBFltr;
	aToB.FltrData = aToBFltrData;
	aToB.P_DeadFlag = &deadFlag;
	aToB.P_NoDataTimeoutTime = &noDatTmoutTime;

	bToA.RSock = sockB;
	bToA.WSock = sockA;
	bToA.DataFltr = bToAFltr;
	bToA.FltrData = bToAFltrData;
	bToA.P_DeadFlag = &deadFlag;
	bToA.P_NoDataTimeoutTime = &noDatTmoutTime;

	aToBTh = runThread(ChannelTransmit, (uint)&aToB);
	bToATh = runThread(ChannelTransmit, (uint)&bToA);

	inner_uncritical();
	{
		waitThread(aToBTh);
		waitThread(bToATh);
	}
	inner_critical();
}
