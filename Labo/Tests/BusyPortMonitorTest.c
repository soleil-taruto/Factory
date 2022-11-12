#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"

// 1 à»è„
static int TimeWaitSec = 120;
// 1 à»è„
static int TimeWaitMax = 5000;
// 0 à»è„
static int TimeWaitRatePower = 2;
// 0 à»è„
static int TimeWaitSleepMillisMax = 1000;

static time_t P_Time;

#define now \
	P_now

static time_t now(void)
{
	return P_Time;
}

// ---- core ----

static int ConnectCount;
static int TimeWaitTotal;
static int *TimeWaitRing;
static time_t LastTime;

static void PCCOrPDC(int connectFlag)
{
	time_t currTime = now();

	errorCase(currTime < LastTime); // 2bs

	if (!TimeWaitRing)
		TimeWaitRing = na_(int, TimeWaitSec);

	while (LastTime < currTime)
	{
		int *tw;

		if (TimeWaitTotal == 0)
		{
			cout("TW_[%I64d]_Break\n", LastTime);

			LastTime = currTime;
			break;
		}
		LastTime++;

		tw = TimeWaitRing + LastTime % TimeWaitSec;

		cout("TW_[%I64d]: %d-%d\n", LastTime, TimeWaitTotal, *tw);

		TimeWaitTotal -= *tw;
		errorCase(TimeWaitTotal < 0); // 2bs
		*tw = 0;
	}
	cout("TW_[%I64d]: %d C=%d\n", LastTime, TimeWaitTotal, ConnectCount);

	if (connectFlag)
	{
		int busyPortTotal = ConnectCount + TimeWaitTotal;
		double rate;
		int waitMillis;

		cout("TW_BUSY: %d\n", busyPortTotal);

		rate = (double)busyPortTotal;
		rate /= TimeWaitMax;
		m_range(rate, 0.0, 1.0);

		cout("TW_RATE: %1.5f\n", rate);

		rate = dPow(rate, TimeWaitRatePower);
		rate *= TimeWaitSleepMillisMax;
		waitMillis = m_d2i(rate);

		cout("TW_WAIT: %d\n", waitMillis);

		Sleep(waitMillis);

		ConnectCount++;
	}
	else
	{
		cout("TW_[%I64d]: %d+1\n", LastTime, TimeWaitTotal);

		TimeWaitTotal++;
		TimeWaitRing[LastTime % TimeWaitSec]++;

		errorCase(ConnectCount <= 0); // êÿífÇÃâÒêîÇ™ëΩÇ∑Ç¨ÇÈÅB
		ConnectCount--;
	}
	cout("TW_%d C=%d\n", TimeWaitTotal, ConnectCount);
}

void PreConnectClient(void)
{
	PCCOrPDC(1);
}
void PostDisconnectClient(void)
{
	PCCOrPDC(0);
}

// ----

int main(int argc, char **argv)
{
	P_Time = 10000;

	if (argIs("/R"))
	{
		mt19937_init();

		while (waitKey(0) != 0x1b)
		{
			int sel = mt19937_rnd(3);

			cout("sel: %d\n", sel);

			switch (sel)
			{
			case 0:
				PreConnectClient();
				break;

			case 1:
				if (ConnectCount) {
					PostDisconnectClient();
				}
				break;

			case 2:
				P_Time++;
				break;
			}
		}
		return;
	}

	for (; ; )
	{
		switch (getKey())
		{
		case 0x1b:
			termination(0);

		case '1':
			PreConnectClient();
			break;

		case '0':
			PostDisconnectClient();
			break;

		case '+':
			P_Time++;
			break;
		}
	}
}
