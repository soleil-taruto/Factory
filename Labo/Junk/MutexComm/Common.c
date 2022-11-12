#include "Common.h"

#define COMMON_ID "{ef807414-f1cd-4ab7-8d83-71ee5e656399}"

static uint Mtxs[M_NUM];
static uint Statuses[M_NUM];

void M_Init(void)
{
	uint index;

	LOGPOS();

	errorCase(Mtxs[0]); // ? already inited

	for (index = 0; index < M_NUM; index++)
		Mtxs[index] = mutexOpen_x(xcout("%s_%u", COMMON_ID, index));

	LOGPOS();
}
void M_Fnlz(void)
{
	uint index;

	LOGPOS();

	errorCase(!Mtxs[0]); // ? already fnlzed

	for (index = 0; index < M_NUM; index++)
	{
		M_Set(index, 0);
		handleClose(Mtxs[index]);
	}
	Mtxs[0] = 0;

	LOGPOS();
}
void M_Set(uint index, uint status)
{
	errorCase(!Mtxs[0]); // ? not inited
	errorCase(M_NUM <= index);
	errorCase(2 <= status);

	if (Statuses[index] != status)
	{
		if (status)
			handleWaitForever(Mtxs[index]);
		else
			mutexRelease(Mtxs[index]);

		Statuses[index] = status;
	}
}
uint M_Get(uint index)
{
	errorCase(!Mtxs[0]); // ? not inited
	errorCase(M_NUM <= index);

	errorCase(Statuses[index]); // 2bs, Ž©•ª‚Å Set ‚µ‚Ä‚¢‚é‚â‚Â‚Í Get ‚µ‚È‚¢‚Í‚¸I

	if (handleWaitForMillis(Mtxs[index], 0))
	{
		mutexRelease(Mtxs[index]);
		return 0;
	}
	return 1;
}
