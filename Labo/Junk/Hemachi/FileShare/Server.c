/*
	想定(必須)オプション
		Adapter
			/X 31000000   (30000000 + margin)
		crypTunnel
			/T 90000      (86400 + margin)
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Labo\Socket\libs\Receptor.h"
#include "libs\all.h"

static int Perform(char *prmFile, char *ansFile)
{
	int retval;

	if (HFS_UnpadFile(prmFile, 0))
	{
		retval = HFS_Perform(prmFile, ansFile);
		HFS_PadFile(ansFile, 1);
	}
	else
	{
		cout("HFS_UnpadFile() FAULT!\n");
		retval = 0;
	}
	return retval;
}
static int Idle(void)
{
	HFS_KeepDiskFreeSpace();

	while (hasKey())
	{
		switch (getKey())
		{
		case 0x1b:
			cout("Exit Proc!\n");
			return 0;

		default:
			cout("Press ESCAPE to exit.\n");
			break;
		}
	}
	return 1;
}
int main(int argc, char **argv)
{
	errorCase(!argIs("/StoreDirAutoClean")); // 2bs

readArgs:
	if (argIs("/DFSM"))
	{
		HFS_DiskFreeSpaceMin = toValue64(nextArg());
		goto readArgs;
	}
	if (argIs("/SDSX"))
	{
		HFS_StoreDirSizeMax = toValue64(nextArg());
		goto readArgs;
	}

	cmdTitle("Hemachi FileShare Server");

	HFS_StoreDir = nextArg();
	HFS_StoreDir = makeFullPath(HFS_StoreDir);
	cout("HFS_StoreDir: %s\n", HFS_StoreDir);

	errorCase(!existDir(HFS_StoreDir));
	errorCase(!mbs_stricmp("C:\\", HFS_StoreDir)); // 2bs

	HFS_UnlockAllPath(); // 不正なロックの解除、念のため。http 動作中だと危険。

	cout("Hemachi FileShare Start\n");
	ReceptorLoopResFile(NULL, Perform, Idle);
	cout("Hemachi FileShare End\n");
}
