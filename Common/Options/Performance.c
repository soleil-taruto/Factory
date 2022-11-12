#include "Performance.h"

uint64 GetPerformanceCounter(void)
{
	LARGE_INTEGER i;

	if (!QueryPerformanceCounter(&i))
	{
		error();
	}
#if 0
	if (i.QuadPart <= 0i64)
	{
		error();
	}
#endif
	return (uint64)i.QuadPart;
}
uint64 GetPerformanceFrequency(void)
{
	static uint64 freq;

	if (freq == 0ui64)
	{
		LARGE_INTEGER i;

		if (!QueryPerformanceFrequency(&i))
		{
			error();
		}
		if (i.QuadPart <= 0i64)
		{
			error();
		}
		freq = (uint64)i.QuadPart;
	}
	return freq;
}
