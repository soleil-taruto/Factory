#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *file1 = getArg(0);
	char *file2 = getArg(1);
	int retval;

	cout("1:%s\n", file1);
	cout("2:%s\n", file2);

	if (isSameFile(file1, file2))
	{
		cout("ˆê’v\n");
		retval = 0;
	}
	else
	{
		cout("•sˆê’v\n");
		retval = 1;
	}
	cout("retval: %d\n", retval);
	termination(retval);
}
