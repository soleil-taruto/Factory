#pragma once

#include "C:\Factory\Common\all.h"

/*
	thread_tls static uint TlsVar;
*/
#define thread_tls \
	__declspec(thread)

uint runThread(void (*userFunc)(uint), uint userInfo);
void waitThread(uint hdl);
int waitThreadEx(uint hdl, uint millis);
void collectDeadThreads(autoList_t *hdls);

typedef struct critical_st // static member
{
	CRITICAL_SECTION Csec;
}
critical_t;

void initCritical(critical_t *i);
void fnlzCritical(critical_t *i);
void enterCritical(critical_t *i);
void leaveCritical(critical_t *i);

void critical(void);
void uncritical(void);
void inner_uncritical(void);
void inner_critical(void);

typedef struct semaphore_st // static member
{
	uint EvLeave;
	uint Count;
}
semaphore_t;

void initSemaphore(semaphore_t *i, uint count);
void fnlzSemaphore(semaphore_t *i);
void enterSemaphore(semaphore_t *i);
void leaveSemaphore(semaphore_t *i);
