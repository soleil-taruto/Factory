#include "SockConnectMon.h"

uint sockConnectMax;
uint sockConnectedCount;
double sockConnectedRate;

void sockConnectMonUpdated(void)
{
	uint level;

	errorCase(!sockConnectMax);
	errorCase(sockConnectMax < sockConnectedCount);

	sockConnectedRate = (double)sockConnectedCount / sockConnectMax;

	     if (0.9 < sockConnectedRate) level = 2;
	else if (0.6 < sockConnectedRate) level = 1;
	else                              level = 0;

	if (level == 2)
		cout("[sockConnectMon] %u / %u = %.6f\n", sockConnectedCount, sockConnectMax, sockConnectedRate);

	if (10 <= sockConnectMax)
	{
		static uint lastLevel;

		if (level != lastLevel)
		{
			static uint lastChanged;
			uint nowTime = now();

			if (lastChanged + 2 < nowTime)
			{
				switch (level)
				{
				case 2: setEmgConsoleColor(0x4f); break;
				case 1: setEmgConsoleColor(0x1f); break;
				case 0: restoreConsoleColor();    break;

				default:
					error();
				}
				lastLevel = level;
				lastChanged = nowTime;
			}
		}
	}
}
