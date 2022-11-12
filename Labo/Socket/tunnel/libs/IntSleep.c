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
void IntSleep(uint millis) // ts_, critical()の中から呼び出さないこと！
{
	INIT();

	// ここから ->

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

	// <- ここまで、不要な気がする。

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
		errorCase(SerialNo == UINT64MAX); // カンスト..有り得ないだろうけど..

		if (UINT64MAX / 2 < SerialNo)
			LOGPOS(); // カンスト注意..有り得ないだろうけど..

		SerialNo++;

		if (WaitCount && !EntryGateLockThHdl)
		{
			eventSet(WakeupEvHdl);
			EntryGateLockThHdl = runThread(EntryGateLockTh, 0);
		}
	}
	uncritical();
}
