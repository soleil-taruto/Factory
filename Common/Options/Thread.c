#include "Thread.h"

typedef struct Info_st
{
	uint Busy;
	void (*UserFunc)(uint);
	uint UserInfo;
}
Info_t;

#define INFO_MAX 3000

static Info_t *Infos;
static uint InfoIndex;

static uint __stdcall Perform(void *prm)
{
	Info_t info = *(Info_t *)prm; // ���̂��R�s�[

	((Info_t *)prm)->Busy = 0; // ���̂��R�s�[�����̂� prm �̎w����͕s�v

	info.UserFunc(info.UserInfo);
	return 0;
}

/*
	�X���b�h�̏I����҂ɂ� waitThread() ���g�p���邱�ƁB
	waitThread(runThread(FuncTh, 0));
	_beginthreadex() �Ő��������X���b�h�͖����I�� CloseHandle() ����K�v������炵���̂ŁAwaitThread() �͕K�{

	�ǂ̊֐����X���b�h�Z�[�t���Ƃ����񂺂�c�����Ă��Ȃ��̂ŁA�����Ƃ��đS�֐��u��X���b�h�Z�[�t�v�Ƃ���B���̊֐����X���b�h�Z�[�t�ł͂Ȃ��B
	�X���b�h�Z�[�t�Ȋ֐��̓R�����g�ɋL�ڂ���B-> // ts_

	��X���b�h�Z�[�t�Ȋ֐����炯�Ȃ̂� userFunc() �́A�ŏ�����Ō�܂� cirtical(), uncritical() �ň͂񂾕������S���낤�B
	�v���v���ňꎞ�I�� inner_uncritical(), inner_critical() ����B

	CritCommonLockCount �� thread_tls �ł��邱�Ƃɒ��ӂ���B
	runThread() ���Ăяo�������� critical() ����Ă���ƁArunThread() ���� uncritical() �����܂� userFunc() ���� critical() �͑҂������B
	userFunc() ���� uncritical() ���Ă� runThread() ���� critical() �̓A�����b�N����Ȃ��I(�X���b�h���Ⴄ����I)
*/
uint runThread(void (*userFunc)(uint), uint userInfo)
{
	Info_t *info;
	uint hdl;

	errorCase(!userFunc);

	if (!Infos)
		Infos = na_(Info_t, INFO_MAX);

	info = Infos + InfoIndex;

	errorCase(info->Busy); // ? INFO_MAX ��O�� Perform() �� info ���܂��Q�Ƃ���Ă��Ȃ��B

	info->Busy = 1;
	info->UserFunc = userFunc;
	info->UserInfo = userInfo;

	InfoIndex = (InfoIndex + 1) % INFO_MAX;

	hdl = (uint)_beginthreadex(0, 0, Perform, info, 0, 0);

	if (hdl == 0) // ? ���s
	{
		error();
	}
	return hdl;
}
void waitThread(uint hdl) // ts_
{
	WaitForSingleObject((HANDLE)hdl, INFINITE);

	critical();
	{
		CloseHandle((HANDLE)hdl);
	}
	uncritical();
}
int waitThreadEx(uint hdl, uint millis) // ts_ ret: ? �I�������B
{
	if (WaitForSingleObject((HANDLE)hdl, millis) != WAIT_TIMEOUT)
	{
		critical();
		{
			CloseHandle((HANDLE)hdl);
		}
		uncritical();
		return 1;
	}
	return 0;
}
void collectDeadThreads(autoList_t *hdls)
{
	uint hdl;
	uint n;

	foreach (hdls, hdl, n)
	{
		if (waitThreadEx(hdl, 0))
		{
			setElement(hdls, n, 0);
		}
	}
	removeZero(hdls);
}

void initCritical(critical_t *i)
{
	InitializeCriticalSection(&i->Csec);
}
void fnlzCritical(critical_t *i)
{
	DeleteCriticalSection(&i->Csec);
}
void enterCritical(critical_t *i)
{
	EnterCriticalSection(&i->Csec);
}
void leaveCritical(critical_t *i)
{
	LeaveCriticalSection(&i->Csec);
}

/*
	critical()
	uncritical()
	inner_uncritical()
	inner_critical()

	�́A�P�v���Z�X���A�ȉ��̏����ŌĂяo����Ȃ���΂Ȃ�Ȃ��B

	ONE-PROCESS :
		critical(); -> ONE-PROCESS -> uncritical();
		| inner_uncritical(); -> ONE-PROCESS -> inner_critical();
		| ""
*/

#define INNER_UNLOCK_MAX 3

static int CritCommonInited;
static critical_t CritCommon;
thread_tls static uint CritCommonLockCount;
thread_tls static uint InnerUnlockCount;
thread_tls static uint InnerLockCountList[INNER_UNLOCK_MAX];

#define CritErrorCase(cond) \
	do { \
	if ((cond)) { \
		critical(); \
		error(); \
	} \
	} while (0)

#if 1
static void CritAfterLeave(void)
{
	sleep(0);
}
#else
#define CritAfterLeave() 1
#endif

void critical(void)
{
	if (!CritCommonLockCount)
	{
		if (!CritCommonInited) // �x���Ƃ��ŏ��� runThread() �Ăяo���O(���ŏ��̃X���b�h�����O)�ɌĂ΂��B<- �������̓N���e�B�J���ɍs����B
		{
			CritCommonInited = 1;
			initCritical(&CritCommon);
		}
		enterCritical(&CritCommon);
	}
	CritCommonLockCount++;
}
void uncritical(void)
{
	CritErrorCase(!CritCommonLockCount);

	CritCommonLockCount--;

	if (!CritCommonLockCount)
	{
		leaveCritical(&CritCommon);
		CritAfterLeave();
	}
}
void inner_uncritical(void)
{
	CritErrorCase(INNER_UNLOCK_MAX <= InnerUnlockCount);

	if (CritCommonLockCount)
	{
		leaveCritical(&CritCommon);
		CritAfterLeave();
	}
	InnerLockCountList[InnerUnlockCount] = CritCommonLockCount;
	InnerUnlockCount++;
	CritCommonLockCount = 0;
}
void inner_critical(void)
{
	CritErrorCase(!InnerUnlockCount);
	CritErrorCase(CritCommonLockCount);

	InnerUnlockCount--;
	CritCommonLockCount = InnerLockCountList[InnerUnlockCount];

	if (CritCommonLockCount)
	{
		enterCritical(&CritCommon);
	}
}

// �ȉ� ts_ �ł͂Ȃ����Ƃɒ��ӁI

void initSemaphore(semaphore_t *i, uint count)
{
	i->EvLeave = eventOpen_x(xcout("{0585e92b-bc63-4521-abdd-bc613fcf691b}_%u", getSelfProcessId()));
	i->Count = count;
}
void fnlzSemaphore(semaphore_t *i)
{
	handleClose(i->EvLeave);
}
void enterSemaphore(semaphore_t *i)
{
	while (!i->Count) // if�ł��ǂ��̂ł́H <-- �_���B
// enter -> !Count�ő҂� -> leave x 2 -> Set����enter������ -> enter -> enter -> !Count�ő҂��ɓ��邪Set����Ă���̂ő҂��Ȃ��B== while�Ŗ߂��Ă���K�v������B
	{
		inner_uncritical();
		{
			collectEvents(i->EvLeave, INFINITE);
		}
		inner_critical();

		if (2 <= i->Count)
			eventSet(i->EvLeave);
	}
	i->Count--;
}
void leaveSemaphore(semaphore_t *i)
{
	if (!i->Count)
	{
		eventSet(i->EvLeave);
	}
	i->Count++;
}
