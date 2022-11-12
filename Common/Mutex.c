#include "all.h"

// typedef void *HANDLE; @ WinNT.h?

/*
	main() {
		uint hdl = mutexLock("MUTEX_NAME");
		���b�N��
		mutexUnlock(hdl);
	}

	main() {
		uint hdl;
		if (hdl = mutexTryLock("MUTEX_NAME")) {
			���b�N��
			mutexUnlock(hdl);
		}
	}

	main() {
		uint hdl = mutexTryProcLock("MUTEX_NAME"); // ���b�N���s -> termination(1);
		���b�N��
		mutexUnlock(hdl);
	}

	main() {
		uint hdl = mutexOpen("MUTEX_NAME");
		handleWaitForever(hdl);
		���b�N��
		mutexRelease(hdl);
		handleClose(hdl);
	}

	main() {
		uint hdl = mutexOpen("MUTEX_NAME");
		if (handleWaitForMillis(hdl, TIMEOUT_MILLIS)) {
			���b�N��
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
		return 1; // �V�O�i����� (���b�N����)
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
int mltHandleWaitForMillis(autoList_t *hdls, uint millis, uint *pIndex) // *pIndex: ret != 0 �̂Ƃ� 0 �` (getCount(hdls) - 1)
{
	uint retval;

	errorCase(!getCount(hdls));

	retval = WaitForMultipleObjects(getCount(hdls), (HANDLE *)directGetList(hdls), FALSE, millis);

	if (WAIT_OBJECT_0 <= retval && retval < WAIT_OBJECT_0 + getCount(hdls))
	{
		if (pIndex)
			*pIndex = retval - WAIT_OBJECT_0;

		return 1; // �V�O�i����� (���b�N����)
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
	�ȒP�Ȏg����

	�e�v���Z�X
		main() {
			uint hdl = eventOpen("cerulean.charlotte Factory test event object");
			�q�v���Z�X�J�n
			eventSleep(hdl); <- �q�v���Z�X�I���҂�
		}

	�q�v���Z�X
		main() {
			����ׂ�����
			eventWakeup("cerulean.charlotte Factory test event object");
		}

	�����n���h���ŉ�����X���[�v����

		uint hdl = eventOpen("test");
		���[�v {
			handleWaitForever(hdl); <- �ʃv���Z�X���͕ʃX���b�h�ł� eventWakeup("test"); �҂�
			����
		}
		handleClose(hdl);

		uint hdl = eventOpen("test");
		���[�v {
			if (handleWaitForMillis(hdl, 500)) {
				����
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
	�C�x���g���
	�}���`�X���b�h�̂Ƃ��AeventSleep() ���N���e�B�J���ɂ���ƃC�x���g�𔭍s���鑤�̃X���b�h�������Ă��܂��̂ŁA�֋X��X���b�h�Z�[�t�Ƃ���B
*/
int collectEvents(uint hdl, uint millis) // ts_
{
	/*
		��̃n���h���ɕ����̃V�O�i�������Ă��~�ς���邱�Ƃ͖����炵���B
		�̂ŏ�̃R�[�h�ŗǂ��͂������ǁA�����������猙�Ȃ̂ŉ��̂܂܂ɂ��Ă����B
		-> �����Ȃ��B@ 2016.10.6
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
