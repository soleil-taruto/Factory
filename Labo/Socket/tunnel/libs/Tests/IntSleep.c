#include "C:\Factory\Common\all.h"
#include "..\IntSleep.h"

static int DeadFlag;

static void SleepTh(uint millis)
{
	for (; ; )
	{
		IntSleep(millis);

		critical();
		{
			cout("WAKEUP %u\n", millis);
		}
		uncritical();
	}
}
int main(int argc, char **argv)
{
	autoList_t *thList = newList();

	critical();
	{
		while (hasArgs(1))
		{
			uint millis = toValue(nextArg());

			cout("THREAD %u\n", millis);
			addElement(thList, runThread(SleepTh, millis));
		}
	}
	uncritical();

	for (; ; )
	{
		int key = getKey();

		critical();
		{
			cout("KEY %02x\n", key);
		}
		uncritical();

		if (key == 0x1b)
			break;

		IntSleepInt();
	}
}
