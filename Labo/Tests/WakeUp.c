/*
	イベント待ちに入る前にwakeupしてもイベントを受け取れるか？ -> 受け取れる。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Thread.h"

static uint Eh;

static void WakeUpTh(uint dummy)
{
	critical();
	{
		LOGPOS();
		eventSet(Eh);
		LOGPOS();
	}
	uncritical();
}
static void WakeUpTh_S(uint dummy)
{
	critical();
	{
		LOGPOS();
	}
	uncritical();

	sleep(2000);

	critical();
	{
		LOGPOS();
		eventSet(Eh);
		LOGPOS();
	}
	uncritical();
}
int main(int argc, char **argv)
{
	uint th;

	/*
		イベント待ちに入る前にwakeupしてもイベントを受け取れるか？
	*/

	LOGPOS();
	Eh = eventOpen("{27e2b893-ca80-4ec0-b293-7c1d4a771f6a}");
	LOGPOS();

	critical();
	{
		LOGPOS();
		th = runThread(WakeUpTh, 0);
		LOGPOS();
	}
	uncritical();

	waitThread(th);
	LOGPOS();
	handleWaitForever(Eh);
	LOGPOS();
	handleClose(Eh);
	LOGPOS();

	/*
		イベント待ち後にwakeup
	*/

	LOGPOS();
	Eh = eventOpen("{54968003-e023-4dcf-8d68-77a30a3e620e}");
	LOGPOS();

	critical();
	{
		LOGPOS();
		th = runThread(WakeUpTh_S, 0);
		LOGPOS();
	}
	uncritical();

	collectEvents(Eh, INFINITE);

	waitThread(th);
	LOGPOS();
	handleClose(Eh);
	LOGPOS();
}
