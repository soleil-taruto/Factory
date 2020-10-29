#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Thread.h"

static void Test2Th(uint ev)
{
	LOGPOS();
	handleWaitForever(ev); // HACK: no crit
	LOGPOS();
}
int main(int argc, char **argv)
{
	uint ev = eventOpen("EvOneHdl-Test");
	uint th;
	uint th1;
	uint th2;
	uint th3;

	// ---- 1 ----

	LOGPOS();
	eventSet(ev);

	LOGPOS();
	handleWaitForever(ev);

	cout("1_OK!\n");

	// ---- 2 ----

	th = runThread(Test2Th, ev);

	sleep(100);

	LOGPOS();
	eventSet(ev); // HACK: no crit

	sleep(100);

	LOGPOS();
	waitThread(th);
	th = 0;

	cout("2_OK!\n");

	// ---- 3 ----

	th1 = runThread(Test2Th, ev);
	sleep(100);
	th2 = runThread(Test2Th, ev);
	sleep(100);
	th3 = runThread(Test2Th, ev);

	sleep(100);

	LOGPOS();
	eventSet(ev); // HACK: no crit

	sleep(100);

	LOGPOS();
	eventSet(ev); // HACK: no crit

	sleep(100);

	LOGPOS();
	eventSet(ev); // HACK: no crit

	sleep(100);

	LOGPOS();
	waitThread(th1);
	waitThread(th2);
	waitThread(th3);
	th1 = 0;
	th2 = 0;
	th3 = 0;

	cout("3_OK!\n");

	// ----

	handleClose(ev);
}
