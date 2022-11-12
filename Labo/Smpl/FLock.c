/*
	FLock.exe (/1 ���b�N�t�@�C�� | /L ���b�N�t�@�C��)

		���b�N�t�@�C�� ... ���b�N����t�@�C��, ���݂���t�@�C���ł��邱�ƁB
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\FLock.h"

static void LockMain(char *file, int retrymax)
{
	FILE *fp;
	uint retrycnt;

	cout("���b�N���܂�...\n");

	for (retrycnt = 0; ; retrycnt++)
	{
		fp = FLock(file);

		if (fp)
			break;

		if (retrymax <= retrycnt)
			break;

		cout("���s���܂����B2�b��Ƀ��g���C���܂��B\n");
		sleep(2000);
	}
	if (!fp)
	{
		cout("���b�N�ł��܂���ł����B\n");
		return;
	}
	cout("���b�N���Ă��܂�...\n");
	getKey();

	FUnlock(fp);

	cout("���b�N�������܂����B\n");
}
int main(int argc, char **argv)
{
	if (argIs("/1"))
	{
		LockMain(nextArg(), 0);
		return;
	}
	if (argIs("/L"))
	{
		LockMain(nextArg(), 10);
		return;
	}
}
