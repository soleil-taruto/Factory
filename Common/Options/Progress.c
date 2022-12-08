#include "Progress.h"

static uint StarCount;

void ProgressBegin(void)
{
	StarCount = 0;
}
void Progress(void)
{
	if (79 <= StarCount)
	{
		uint index;

		cout("\r");

		for (index = 0; index < 79; index++)
		{
			cout("%c", (StarCount - index / 20) & 2 ? '*' : '-');
		}
	}
	else
	{
		cout("*");
	}
	StarCount++;
}
void ProgressRate(double rate) // rate 0.0 - 1.0
{
	uint end = m_d2i(rate * 79.0);

	m_range(end, 0, 79);

	while (StarCount < end)
	{
		Progress();
//		sleep(0);
//		sleep(1);
	}
}
void ProgressEnd(int cancelled)
{
	if (cancelled)
		cout("\r-------------------------------------------------------------------------------\n");
	else
		cout("\r*******************************************************************************\n");
}
