#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Thread.h"

static int KeepTh;
static CRITICAL_SECTION CrSec;

static void HeavyProc(void)
{
	uint c;

	for (c = 10000000; c; c--)
	{
		noop();
	}
}

#define FUNC(funcname, sleep_op) \
	static void funcname(uint dummy) { \
		uint c, d; \
		EnterCriticalSection(&CrSec); \
		for (c = 1000; c && KeepTh; c--) { \
			cout("."); \
			HeavyProc(); \
			LeaveCriticalSection(&CrSec); \
			sleep_op; \
			EnterCriticalSection(&CrSec); \
		} \
		LeaveCriticalSection(&CrSec); \
	}

FUNC(Th_01, Sleep(0)) // MSDN
FUNC(Th_02, sleep(0)) // my sleep, will be context switching

#undef FUNC

static void Test01(void (*funcTh)(uint))
{
	uint th;
	uint c;

	KeepTh = 1;

	EnterCriticalSection(&CrSec);
	{
		th = runThread(funcTh, 0);
	}
	LeaveCriticalSection(&CrSec);

	for (c = 0; c < 30; c++)
	{
		Sleep(1); // switch to Th_0x

		EnterCriticalSection(&CrSec);
		{
			cout("!");
		}
		LeaveCriticalSection(&CrSec);
	}
	KeepTh = 0;
	waitThread(th);

	cout("\n");
}
int main(int argc, char **argv)
{
	InitializeCriticalSection(&CrSec);

	cout("MSDN\n");
	Test01(Th_01);
	cout("My sleep(), will be context switching\n");
	Test01(Th_02);

	DeleteCriticalSection(&CrSec);
}
