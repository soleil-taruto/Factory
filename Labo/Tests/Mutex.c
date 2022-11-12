#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *name = "abc";

readArgs:
	if (argIs("/N"))
	{
		name = nextArg();
		goto readArgs;
	}

	if (argIs("/K"))
	{
		uint mtx;

		LOGPOS();
		mtx = mutexLock(name);
		LOGPOS();
		{
			cout("KEY_WAIT...\n");
			getKey();
			cout("KEY_PRESSED\n");
		}
		LOGPOS();
		mutexUnlock(mtx);
		LOGPOS();
		return;
	}
	if (argIs("/T"))
	{
		uint millis = toValue(nextArg());
		uint mtx;

		LOGPOS();
		mtx = mutexLock(name);
		LOGPOS();
		{
			coSleep(millis);
		}
		LOGPOS();
		mutexUnlock(mtx);
		LOGPOS();
		return;
	}
}
