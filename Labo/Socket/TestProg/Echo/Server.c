#include "C:\Factory\Common\Options\SockServer.h"

static int Perform(char *prmFile, char *ansFile)
{
	cout("prmFile %s\n", prmFile);
	cout("ansFile %s\n", ansFile);

#if 1
	removeFile(ansFile);
	moveFile(prmFile, ansFile);
	createFile(prmFile);
#else
	copyFile(prmFile, ansFile);
#endif

	cout("Perform() OK\n");
	return 1;
}
static int Idle(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("�I��\n");
			return 0;
		}
		else
		{
			cout("ESCAPE == �I��\n");
		}
	}
	return 1;
}
int main(int argc, char **argv)
{
	sockServer(Perform, hasArgs(1) ? toValue(nextArg()) : 65000, 10, 0x7fffffffffffffffui64, Idle);
}
