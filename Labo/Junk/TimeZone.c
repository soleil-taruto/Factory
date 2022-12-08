#include "C:\Factory\Common\all.h"

static sint GetTimeZone(void)
{
	stampData_t *i = getStampDataTime(86400L);
	sint ret;

	ret = i->hour * 3600 + i->minute * 60 + i->second;

	if (i->day == 1)
		ret -= 86400;

	return ret;
}
static char *GetStrTimeZone(void)
{
	sint tz = GetTimeZone();
	sint sign;
	uint m;
	uint s;

	if (tz < 0)
	{
		tz *= -1;
		sign = -1;
	}
	else
	{
		sign = 1;
	}
	s = tz % 60;
	tz /= 60;
	m = tz % 60;
	tz /= 60;

	{
		char *ret = xcout("UTC%c%d:%02u:%02u", sign == -1 ? '-' : '+', tz, m, s);

		if (!s)
		{
			ret[strlen(ret) - 3] = '\0';

			if (!m)
				ret[strlen(ret) - 3] = '\0';
		}
		return ret;
	}
}
int main(int argc, char **argv)
{
	cout("%s\n", GetStrTimeZone()); // g
}
