#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

static void CRCatnap(uint millisLmt)
{
	uint millis = mt19937_rnd(millisLmt);

	cout("millis: %u\n", millis);
	sleep(millis);
}
int main(int argc, char **argv)
{
	mt19937_initCRnd();

	if (hasArgs(1))
	{
		CRCatnap(toValue(nextArg()));
	}
	else
	{
		CRCatnap(1000);
	}
}
