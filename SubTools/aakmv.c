/*
	MOVE�R�}���h�Ńt�@�C�����ړ�����B
	�A�N�Z�X���ۂȂǂŎ��s���Ă���������܂Ń��g���C����B

	aakmv.exe [/R RETRY-MAX | /RX] SRC-FILE DEST-PATH

		RETRY-MAX ... ���g���C�񐔂̏�� 0 �` UINTMAX, def 10
		/RX       ... ���g���C�� UINTMAX �� <- �قډi�v���g���C
		SRC-FILE  ... �ړ����̃t�@�C��
		DEST-PATH ... �ړ���̃t�@�C�����̓f�B���N�g��
*/

#include "C:\Factory\Common\all.h"

static uint RetryMax = 10; // 0 �` UINTMAX

static void AAKMove(char *srcFile, char *destPath)
{
	char *destFile;
	uint retrycnt;

	srcFile = makeFullPath(srcFile);
	destPath = makeFullPath(destPath);

	cout("< %s\n", srcFile);
	cout("! %s\n", destPath);

	if (existDir(destPath))
		destFile = combine(destPath, getLocal(srcFile));
	else
		destFile = strx(destPath);

	cout("> %s\n", destFile);

	removeFileIfExist(destFile);

	for (retrycnt = 0; ; retrycnt++)
	{
		coExecute_x(xcout("MOVE \"%s\" \"%s\"", srcFile, destFile));

		if (existFile(destFile))
			break;

		errorCase(RetryMax <= retrycnt);

		coSleep(2000);
	}

	memFree(srcFile);
	memFree(destPath);
	memFree(destFile);
}
int main(int argc, char **argv)
{
	if (argIs("/R"))
	{
		RetryMax = toValue(nextArg());
	}
	if (argIs("/RX"))
	{
		RetryMax = UINTMAX;
	}

	AAKMove(getArg(0), getArg(1));
}
