#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	setAllProcPoundKey(0x1b);
	ignoreAllProcPoundKey = 1;
	coWaitKey(15000);
	setAllProcPoundKey(0);
}
