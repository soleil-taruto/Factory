// smpl
#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Thread.h"
#include "C:\Factory\Common\Options\CRRandom.h"

#define TH_NUM 50
#define SMPH_MAX 20

static uint Ths[TH_NUM];
static semaphore_t Smph;
static int Dead;
static uint EnterCount;

static void RndSleep(void)
{
	uint millis = mt19937_rnd(10);

	if (millis)
	{
		inner_uncritical();
		{
			sleep(millis);
		}
		inner_critical();
	}
}
static void Th(uint index)
{
	critical();
	{
		for (; ; )
		{
			enterSemaphore(&Smph);
			{
				cout("+ %u\n", EnterCount);
				EnterCount++;
				cout("> %u\n", EnterCount);

				RndSleep();

				cout("- %u\n", EnterCount);
				EnterCount--;
				cout("> %u\n", EnterCount);
			}
			leaveSemaphore(&Smph);

			if (Dead || waitKey(0) == 0x1b)
			{
				Dead = 1;
				break;
			}
			RndSleep();
		}
	}
	uncritical();
}
static void SemaphoreMain(void)
{
	uint index;

	mt19937_initCRnd();

	initSemaphore(&Smph, SMPH_MAX);

	critical();
	{
		for (index = 0; index < TH_NUM; index++)
		{
			Ths[index] = runThread(Th, index);
		}
	}
	uncritical();

	for (index = 0; index < TH_NUM; index++)
	{
		waitThread(Ths[index]);
	}
	fnlzSemaphore(&Smph);
}
int main(int argc, char **argv)
{
	SemaphoreMain();
}
