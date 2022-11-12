/*
	�t�@�C���E�f�B���N�g���̃R�s�[

	----

	zcp.exe [/F] [/M | /R] [/EF] [/B] SOURCE-PATH DESTINATION-PATH

		/F ... �������[�h

			�R�s�[�悪���ɑ��݂���ꍇ�A�₢���킹���邱�ƂȂ��폜����B

		/M, /R ... �ړ����[�h

			�R�s�[�����폜����B�܂�ړ�����B

		/EF ... �g���q�t�B���^

			�R�s�[��̃t�@�C���E�f�B���N�g�����̊g���q�I�[�� '_' ����������B

		/B ... �R�s�[��̐e�f�B���N�g�����w�肷��B

			�ȉ��͓���
			zcp /B �T���v���f�[�^ out
			zcp �T���v���f�[�^ out\�T���v���f�[�^
*/

#include "C:\Factory\Common\all.h"

static void E_Fltr(char *path)
{
	char *ext = getExt(path);

	if (lineExp("<1,,09AZaz>_", ext))
	{
		char *newPath = strx(path);

		strchr(newPath, '\0')[-1] = '\0';

		cout("< %s\n", path);
		cout("> %s\n", newPath);

		moveFile(path, newPath);
		memFree(newPath);
	}
}
int main(int argc, char **argv)
{
	char *srcPath;
	char *destPath;
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
	if (argIs("/EF"))
	{
		ef_mode = 1;
		goto readArgs;
	}
	if (argIs("/B"))
	{
		b_mode = 1;
		goto readArgs;
	}

	srcPath  = nextArg();
	destPath = nextArg();

	/*
		�I�v�V�������ԈႦ���H -> �O�̂��� error
	*/
	errorCase(srcPath[0]  == '/');
	errorCase(destPath[0] == '/');
	errorCase(hasArgs(1));

	srcPath  = makeFullPath(srcPath);
	destPath = makeFullPath(destPath);

	if (b_mode)
		destPath = addLocal(destPath, getLocal(srcPath));

	cout("< %s\n", srcPath);
	cout("> %s\n", destPath);

	if (existPath(destPath))
	{
		if (!force_mode)
		{
			cout("�R�s�[�悪���݂��܂��B\n");
			cout("�폜�H\n");

			if (clearGetKey() == 0x1b)
				termination(0);

			cout("�폜���܂��B\n");
		}
		recurRemovePath(destPath);
	}

	if (existFile(srcPath))
	{
		createPath(destPath, 'X');

		if (move_mode)
		{
			moveFile(srcPath, destPath);
		}
		else
		{
			copyFile(srcPath, destPath);
		}
	}
	else
	{
		errorCase(!existDir(srcPath));
		createPath(destPath, 'D');

		if (move_mode)
		{
			moveDir(srcPath, destPath);
			removeDir(srcPath);
		}
		else
		{
			copyDir(srcPath, destPath);
		}
	}

	if (ef_mode)
	{
		cout("ef=%s\n", destPath);

		if (existFile(destPath))
		{
			E_Fltr(destPath);
		}
		else
		{
			autoList_t *paths = lss(destPath);
			char *path;
			uint index;

			reverseElements(paths);

			foreach (paths, path, index)
			{
				E_Fltr(path);
			}
			releaseDim(paths, 1);
		}
	}

	memFree(srcPath);
	memFree(destPath);
}
