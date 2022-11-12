#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	uint h;
	uint ret = 0;

	h = getStampData(makeStamp(0))->hour;
	cout("h: %u\n", h);

	if (h == 5)
		ret = 1;

	cout("ret: %u\n", ret);
	termination(ret);
}
