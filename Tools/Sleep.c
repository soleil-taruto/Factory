#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	sleep(toValue(nextArg()) & 0xffff);
	return hasKey() ? 1 : 0;
}
