/*
	Lock.exe ロックファイル (LOCK | UNLOCK)
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *lockingFile = nextArg();

	lockingFile = makeFullPath(lockingFile);

	cout("ロックファイル：%s\n", lockingFile);

	if (argIs("LOCK"))
	{
		cout("ロックしています...\n");
		LOGPOS_T();

		for (; ; )
		{
			int locked = 0;

			mutex();
			{
				if (!existFile(lockingFile))
				{
					createFile(lockingFile);
					locked = 1;
				}
			}
			unmutex();

			if (locked)
				break;

			sleep(2000);
		}
		LOGPOS_T();
		cout("ロックしました。\n");
	}
	else if (argIs("UNLOCK"))
	{
		cout("ロック解除しています...\n");

		mutex();
		{
			removeFileIfExist(lockingFile);
		}
		unmutex();

		cout("ロック解除しました。\n");
		LOGPOS_T();
	}
	else
	{
		error();
	}
}
