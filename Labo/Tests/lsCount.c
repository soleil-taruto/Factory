#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	uint count;

	LOGPOS();
	count = lsCount(nextArg());
	LOGPOS();
	cout("%u\n", count);
}
