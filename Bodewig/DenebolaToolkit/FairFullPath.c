#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *ret = toFairFullPathFltr(nextArg());

	cout("%s%s\n", majorOutputLinePrefix, ret);

	memFree(ret);
}
