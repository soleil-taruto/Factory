/*
	Lock.exe ���b�N�t�@�C�� (LOCK | UNLOCK)
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *lockingFile = nextArg();

	lockingFile = makeFullPath(lockingFile);

	cout("���b�N�t�@�C���F%s\n", lockingFile);

	if (argIs("LOCK"))
	{
		cout("���b�N���Ă��܂�...\n");
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
		cout("���b�N���܂����B\n");
	}
	else if (argIs("UNLOCK"))
	{
		cout("���b�N�������Ă��܂�...\n");

		mutex();
		{
			removeFileIfExist(lockingFile);
		}
		unmutex();

		cout("���b�N�������܂����B\n");
		LOGPOS_T();
	}
	else
	{
		error();
	}
}
