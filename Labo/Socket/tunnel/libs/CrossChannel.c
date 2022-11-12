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
int ChannelDeadFlag; // tls�ł͂Ȃ��̂ŁADataFltr���s���ɃX���b�h���؂�ւ������ŕύX����邩������Ȃ����ɒ��ӁI

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

			if (getSize(wBuff) < 1000000) // ? ! ���M�o�b�t�@���܂�߂�
			{
				int execDataFltrFlag = 0;

				if (getSize(wBuff))
					rTmout = 0;

				/*
					�Е��̃\�P�b�g���ؒf���ꂽ���A�����Е��ւ� SockRecvSequ() ���� select() �͑����ɏI���Ȃ��B
					-> �҂����Ԃ𒲐�����悤�ɂ����B
					�ؒf���ꂽ���ւ� select() �����̂������ɏI���Ȃ��悤�Ɍ������B
					�ؒf�̌��m�� select() �͖�肠��Ȃ̂�������Ȃ��B
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
				�t������ SockSendSequ() �� -1 ��Ԃ���(i->RSock ���ؒf����)�ꍇ�ł� i->RSock �ɓǂݍ��݉\�ȃf�[�^�����邩������Ȃ��H
				����Ƃ���� i->WSock �͐����Ă邩������Ȃ��̂ŁA�ǂݍ��߂Ȃ��Ȃ�܂� SockRecvSequ() �����s�������B
				-> i->P_DeadFlag �̗����オ��ő��I�����Ȃ��悤�ɂ����B
			*/
			if (*i->P_DeadFlag) // ? �t�����Őؒf�����m�����B
			{
				if (dosFlag)
				{
					if (getSize(wBuff) == 0) // ? ��M�f�[�^�E���M�f�[�^���ɖ����̂ŏI��
						goto endTransmit;

					if (dosTmout < now()) // ? �^�C���A�E�g
					{
						cout("�������t�����̐ؒf�����m�����̂Ƀ^�C���A�E�g_wBuff_size: %u @ %s\n", getSize(wBuff), tmp = makeJStamp(NULL, 0));
						memFree(tmp);
						goto endTransmit;
					}
					cout("������CC_T_R %u @ %s\n", getSize(wBuff), tmp = makeJStamp(NULL, 0));
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
				cout("������CC���ʐM�^�C���A�E�g������\n");
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

				if (abortTime < now()) // ? �^�C���A�E�g
				{
					cout("�������������݃^�C���A�E�g_wBuff_size: %u @ %s\n", getSize(wBuff), tmp = makeJStamp(NULL, 0));
					memFree(tmp);
					break;
				}
				cout("������CC_T_S %u @ %s\n", getSize(wBuff), tmp = makeJStamp(NULL, 0));
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
