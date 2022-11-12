#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\OpenSource\mt19937.h"

static int KeepServerRq = 1;

static int Perform(char *prmFile, char *ansFile)
{
	uint count = mt19937_rnd(300);

	cout("Perform Start\n");

	removeFile(ansFile);
	moveFile(prmFile, ansFile);
	createFile(prmFile);

	while (count)
	{
		cout("%u\n", count);

		while (hasKey())
		{
			if (getKey() == 0x1b)
			{
				cout("Perform Abort\n");
				KeepServerRq = 0;
				return 0;
			}
		}

		cout("Interrupt Start\n");
		sockServerPerformInterrupt();
		cout("Interrupt End\n");

		sleep(1000);
		count--;
	}
	cout("Perform End\n");
	return 1;
}
static int Idle(void)
{
	while (hasKey())
		if (getKey() == 0x1b)
			KeepServerRq = 0;

	return KeepServerRq;
}
int main(int argc, char **argv)
{
	sockServer(Perform, 65000, 10, UINT64MAX, Idle);
}
