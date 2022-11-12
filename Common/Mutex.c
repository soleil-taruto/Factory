#include "all.h"

// typedef void *HANDLE; @ WinNT.h?

/*
	main() {
		uint hdl = mutexLock("MUTEX_NAME");
		ロック中
		mutexUnlock(hdl);
	}

	main() {
		uint hdl;
		if (hdl = mutexTryLock("MUTEX_NAME")) {
			ロック中
			mutexUnlock(hdl);
		}
	}

	main() {
		uint hdl = mutexTryProcLock("MUTEX_NAME"); // ロック失敗 -> termination(1);
		ロック中
		mutexUnlock(hdl);
	}

	main() {
		uint hdl = mutexOpen("MUTEX_NAME");
		handleWaitForever(hdl);
		ロック中
		mutexRelease(hdl);
		handleClose(hdl);
	}

	main() {
		uint hdl = mutexOpen("MUTEX_NAME");
		if (handleWaitForMillis(hdl, TIMEOUT_MILLIS)) {
			ロック中
			mutexRelease(hdl);
		}
		handleClose(hdl);
	}
*/

uint mutexOpen(char *mutexName)
{
	HANDLE hdl;

//	mutexName = xcout("Global\\%s", mutexName);
	hdl = CreateMutexA(NULL, FALSE, mutexName);
//	memFree(mutexName);

	if (hdl == NULL)
	{
		error();
	}
	return (uint)hdl;
}
void handleClose(uint hdl)
{
	if (CloseHandle((HANDLE)hdl) == 0)
	{
		error();
	}
}

int handleWaitForMillis(uint hdl, uint millis)
{
	uint retval = WaitForSingleObject((HANDLE)hdl, millis);

	if (retval == WAIT_OBJECT_0)
	{
		return 1; // シグナル状態 (ロック成功)
	}
	if (retval == WAIT_TIMEOUT)
	{
		return 0;
	}
	error();
	return 0; // Dummy
}
void handleWaitForever(uint hdl)
{
	if (!handleWaitForMillis(hdl, INFINITE))
	{
		error();
	}
}
int mltHandleWaitForMillis(autoList_t *hdls, uint millis, uint *pIndex) // *pIndex: ret != 0 のとき 0 ～ (getCount(hdls) - 1)
{
	uint retval;

	errorCase(!getCount(hdls));

	retval = WaitForMultipleObjects(getCount(hdls), (HANDLE *)directGetList(hdls), FALSE, millis);

	if (WAIT_OBJECT_0 <= retval && retval < WAIT_OBJECT_0 + getCount(hdls))
	{
		if (pIndex)
			*pIndex = retval - WAIT_OBJECT_0;

		return 1; // シグナル状態 (ロック成功)
	}
	if (retval == WAIT_TIMEOUT)
	{
		return 0;
	}
	error();
	return 0; // Dummy
}
void mltHandleWaitForever(autoList_t *hdls, uint *pIndex)
{
	if (!mltHandleWaitForMillis(hdls, INFINITE, pIndex))
	{
		error();
	}
}
void mutexRelease(uint hdl)
{
	if (ReleaseMutex((HANDLE)hdl) == 0)
	{
		error();
	}
}

uint mutexLock(char *mutexName)
{
	uint hdl = mutexOpen(mutexName);

	handleWaitForever(hdl);
	return hdl;
}
uint mutexTryLock(char *mutexName)
{
	uint hdl = mutexOpen(mutexName);

	if (handleWaitForMillis(hdl, 0))
		return hdl;

	handleClose(hdl);
	return 0;
}
uint mutexTryProcLock(char *mutexName)
{
	uint hdl = mutexTryLock(mutexName);

	if (!hdl)
		termination(1);

	return hdl;
}
void mutexUnlock(uint hdl)
{
	mutexRelease(hdl);
	handleClose(hdl);
}

#define COMMONMUTEXNAME "cerulean.charlotte Factory common mutex object"

static uint CommonHandle;
static uint CommonLockCount;

void mutex(void)
{
	if (!CommonLockCount)
	{
		CommonHandle = mutexLock(COMMONMUTEXNAME);
	}
	CommonLockCount++;
}
void unmutex(void)
{
	errorCase(!CommonLockCount);

	CommonLockCount--;
	if (!CommonLockCount)
	{
		mutexUnlock(CommonHandle);
	}
}

/*
	簡単な使い方

	親プロセス
		main() {
			uint hdl = eventOpen("cerulean.charlotte Factory test event object");
			子プロセス開始
			eventSleep(hdl); <- 子プロセス終了待ち
		}

	子プロセス
		main() {
			するべき処理
			eventWakeup("cerulean.charlotte Factory test event object");
		}

	同じハンドルで何回もスリープする

		uint hdl = eventOpen("test");
		ループ {
			handleWaitForever(hdl); <- 別プロセス又は別スレッドでの eventWakeup("test"); 待ち
			処理
		}
		handleClose(hdl);

		uint hdl = eventOpen("test");
		ループ {
			if (handleWaitForMillis(hdl, 500)) {
				処理
			}
		}
		handleClose(hdl);
*/

uint eventOpen(char *eventName)
{
	HANDLE hdl = CreateEventA(NULL, FALSE, FALSE, eventName);

	if (hdl == NULL)
	{
		error();
	}
	return (uint)hdl;
}
void eventSet(uint hdl)
{
	if (SetEvent((HANDLE)hdl) == 0)
	{
		error();
	}
}

void eventWakeupHandle(uint hdl)
{
	eventSet(hdl);
	handleClose(hdl);
}
void eventWakeup(char *eventName)
{
	eventWakeupHandle(eventOpen(eventName));
}
void eventSleep(uint hdl)
{
	handleWaitForever(hdl);
	handleClose(hdl);
}

/*
	イベント回収
	マルチスレッドのとき、eventSleep() をクリティカルにするとイベントを発行する側のスレッドが凍ってしまうので、便宜上スレッドセーフとする。
*/
int collectEvents(uint hdl, uint millis) // ts_
{
	/*
		一つのハンドルに複数のシグナルが来ても蓄積されることは無いらしい。
		ので上のコードで良いはずだけど、何かあったら嫌なので下のままにしておく。
		-> おかない。@ 2016.10.6
	*/
#if 1
	return WaitForSingleObject((HANDLE)hdl, millis) == WAIT_OBJECT_0 ? 1 : 0;
#else
	uint scnt = 0;

	while (WaitForSingleObject((HANDLE)hdl, millis) == WAIT_OBJECT_0)
	{
		millis = 0;
		scnt++;
	}
	return (int)scnt;
#endif
}

// _x
uint mutexOpen_x(char *mutexName)
{
	uint out = mutexOpen(mutexName);
	memFree(mutexName);
	return out;
}
uint eventOpen_x(char *eventName)
{
	uint out = eventOpen(eventName);
	memFree(eventName);
	return out;
}
