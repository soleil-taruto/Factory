#include "IntSleep.h"

static critical_t EntryGate;
static uint WakeupEvHdl;
static uint EntryGateUnlockEvHdl;
static uint64 SerialNo;
thread_tls static uint64 ThSerialNo;
static uint WaitCount;
static uint EntryGateLockThHdl;

static void INIT(void)
{
	static int inited;

	if (!inited)
	{
		critical();
		{
			if (!inited)
			{
				initCritical(&EntryGate);
				WakeupEvHdl = eventOpen_x(MakeUUID(1));
				EntryGateUnlockEvHdl = eventOpen_x(MakeUUID(1));
				inited = 1;
			}
		}
		uncritical();
	}
}
void IntSleep(uint millis) // ts_, critical()�̒�����Ăяo���Ȃ����ƁI
{
	INIT();

	// �������� ->

	critical();
	{
		if (ThSerialNo != SerialNo)
		{
			ThSerialNo = SerialNo;
			uncritical();
			return;
		}
	}
	uncritical();

	// <- �����܂ŁA�s�v�ȋC������B

	enterCritical(&EntryGate);
	leaveCritical(&EntryGate);

	critical();
	{
		if (ThSerialNo != SerialNo)
		{
			ThSerialNo = SerialNo;
			uncritical();
			return;
		}
		WaitCount++;
	}
	uncritical();

	collectEvents(WakeupEvHdl, millis);

	critical();
	{
		if (ThSerialNo != SerialNo)
			ThSerialNo = SerialNo;

		WaitCount--;

		if (EntryGateLockThHdl)
		{
			if (WaitCount)
			{
				eventSet(WakeupEvHdl);
			}
			else
			{
				eventSet(EntryGateUnlockEvHdl);
				waitThread(EntryGateLockThHdl);
				EntryGateLockThHdl = 0;
			}
		}
	}
	uncritical();
}
static void EntryGateLockTh(uint dummy)
{
	enterCritical(&EntryGate);
	collectEvents(EntryGateUnlockEvHdl, INFINITE);
	leaveCritical(&EntryGate);
}
void IntSleepInt(void) // ts_
{
	INIT();

	critical();
	{
		errorCase(SerialNo == UINT64MAX); // �J���X�g..�L�蓾�Ȃ����낤����..

		if (UINT64MAX / 2 < SerialNo)
			LOGPOS(); // �J���X�g����..�L�蓾�Ȃ����낤����..

		SerialNo++;

		if (WaitCount && !EntryGateLockThHdl)
		{
			eventSet(WakeupEvHdl);
			EntryGateLockThHdl = runThread(EntryGateLockTh, 0);
		}
	}
	uncritical();
}
