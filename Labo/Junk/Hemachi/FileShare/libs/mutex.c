#include "all.h"

#define MUTEXNAME "cerulean.charlotte Factory Hemachi FileShare mutex object"

static uint MutexHandle;

void HFS_MutexEnter(void)
{
	errorCase(MutexHandle); // ? �v���Z�X���̑��d���b�N

	cout("HFS_MutexEnter() Start\n");
	MutexHandle = mutexLock(MUTEXNAME);
	cout("HFS_MutexEnter() OK!\n");
}
void HFS_MutexLeave(void)
{
	errorCase(!MutexHandle); // ? ���b�N���Ă��Ȃ��B

	cout("HFS_MutexLeave() Start\n");
	mutexUnlock(MutexHandle);
	cout("HFS_MutexLeave() OK!\n");

	MutexHandle = 0;
}
