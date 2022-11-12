#include "TimeWaitMonitor.h"

#define TIME_WAIT_SEC 120
#define TIME_WAIT_MAX 5000
#define TIME_WAIT_RATE_POWER 2
#define TIME_WAIT_SLEEP_MILLIS_MAX 1000

static uint LastTime;
static uint TimeWaitTotal;
static uint TimeWaitRing[TIME_WAIT_SEC];

void AddTimeWait(void)
{
	uint currTime = now();

	errorCase(currTime < LastTime); // 2bs

	while (LastTime < currTime)
	{
		if (!TimeWaitTotal)
		{
			cout("TW_BRK\n");

			LastTime = currTime;
			break;
		}

		LastTime++;

		if (TimeWaitRing[LastTime % TIME_WAIT_SEC])
			cout("TW_[%u]: %u-%u\n", LastTime, TimeWaitTotal, TimeWaitRing[LastTime % TIME_WAIT_SEC]);

		TimeWaitTotal -= TimeWaitRing[LastTime % TIME_WAIT_SEC];
		TimeWaitRing[LastTime % TIME_WAIT_SEC] = 0;
	}

	cout("TW_[%u]: %u+1\n", LastTime, TimeWaitTotal);

	TimeWaitTotal++;
	TimeWaitRing[LastTime % TIME_WAIT_SEC]++;

	{
		double rate;
		uint waitMillis;

		rate = (double)TimeWaitTotal;
		rate /= TIME_WAIT_MAX;
		m_range(rate, 0.0, 1.0);

		cout("TW_RATE: %1.6f\n", rate);

		rate = dPow(rate, TIME_WAIT_RATE_POWER);
		cout("TW_RAT2: %1.6f\n", rate);
		rate *= TIME_WAIT_SLEEP_MILLIS_MAX;
		cout("TW_RAT3: %1.6f\n", rate);

		waitMillis = m_d2i(rate);

		cout("TW_WAIT: %u\n", waitMillis);

		sleep(waitMillis);
	}
}
