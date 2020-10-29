#include "Lock.h"

#define MUTEX_COMMON "{cfd0b3aa-7a82-4d42-9a45-3fbfe8fdf06a}"

static uint MtxCmn;

void BlueFish_Lock(void)
{
	if(!IsBlueFishComputer())
	{
		LOGPOS();
		return;
	}

	LOGPOS();

	errorCase(MtxCmn);
	MtxCmn = mutexLock(MUTEX_COMMON);

	LOGPOS();
}
void BlueFish_Unlock(void)
{
	if(!IsBlueFishComputer())
	{
		LOGPOS();
		return;
	}

	LOGPOS();

	errorCase(!MtxCmn);
	mutexUnlock(MtxCmn);
	MtxCmn = 0;

	LOGPOS();
}
