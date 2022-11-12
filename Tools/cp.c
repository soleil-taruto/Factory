/*
	�t�@�C���̃R�s�[

	----

	cp.exe SOURCE-FILE DESTINATION-PATH

		/F ... �������[�h

			�R�s�[�悪���ɑ��݂���ꍇ�A�₢���킹���邱�ƂȂ��폜����B

		/M, /R ... �ړ����[�h

			�R�s�[�����폜����B�܂�ړ�����B
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *srcFile;
	char *destFile;
	int force_mode = 0;
	int move_mode = 0;
	int ef_mode = 0;
	int b_mode = 0;

readArgs:
	if (argIs("/F"))
	{
		force_mode = 1;
		goto readArgs;
	}
	if (argIs("/M") || argIs("/R"))
	{
		move_mode = 1;
		goto readArgs;
	}

	srcFile  = nextArg();
	destFile = nextArg();

	/*
		�I�v�V�������ԈႦ���H -> �O�̂��� error
	*/
	errorCase(srcFile[0]  == '/');
	errorCase(destFile[0] == '/');
	errorCase(hasArgs(1));

	srcFile  = makeFullPath(srcFile);
	destFile = makeFullPath(destFile);

	errorCase(!existFile(srcFile));

	if (existDir(destFile))
	{
		destFile = addLocal(destFile, getLocal(srcFile));

		errorCase(existDir(destFile));
	}

	cout("< %s\n", srcFile);
	cout("> %s\n", destFile);

	if (existFile(destFile))
	{
		if (!force_mode)
		{
			cout("�R�s�[��t�@�C�������݂��܂��B\n");
			cout("�폜�H\n");

			if (clearGetKey() == 0x1b)
				termination(0);

			cout("�폜���܂��B\n");
		}
		recurRemovePath(destFile);
	}

	if (move_mode)
	{
		moveFile(srcFile, destFile);
	}
	else
	{
		copyFile(srcFile, destFile);
	}

	memFree(srcFile);
	memFree(destFile);
}
