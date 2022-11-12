/*
	LotCopy.exe [�R�s�[��DIR �R�s�[��eDIR]

		�R�s�[��DIR   ... ���݂���f�B���N�g���ł��邱�ƁB
		�R�s�[��eDIR ... ���݂���f�B���N�g���ł��邱�ƁB

		�R�s�[��eDIR�̒����ɃR�s�[��DIR�̃��[�J�����̃f�B���N�g�����쐬���āA�����փR�s�[����B
		�R�s�[�悪�d�������ꍇ�A���O���G�X�P�[�v����B
		�񓯊��ŃR�s�[����B

	LotCopy.exe /D [�R�s�[��eDIR]

		�R�s�[��eDIR ... ���݂���f�B���N�g���ł��邱�ƁB

		�R�s�[��eDIR���Œ肵�āA�����̃R�s�[��DIR���w�肷��B
*/

#include "C:\Factory\Common\all.h"

static void DoCopy(char *rDir, char *wDir)
{
	rDir = makeFullPath(rDir);
	wDir = makeFullPath(wDir);

	cout("0.< %s\n", rDir);
	cout("0.> %s\n", wDir);

	errorCase_m(!existDir(rDir), "�R�s�[���f�B���N�g���͑��݂��܂���B");
	errorCase_m(!existDir(wDir), "�R�s�[��f�B���N�g��(���[�g)�͑��݂��܂���B");

	{
		char *rLDir = getLocal(rDir);

		if (*rLDir)
			rLDir = strx(rLDir);
		else
			rLDir = xcout("%c$$", rDir[0]);

		wDir = combine_xx(wDir, rLDir);
		wDir = toCreatableTildaPath(wDir, IMAX);
	}

	cout("1.> %s\n", wDir);

	createDir(wDir);
	coExecute_x(xcout("START \"\" /MIN ROBOCOPY.EXE \"%s\" \"%s\" /MIR", rDir, wDir));

	memFree(rDir);
	memFree(wDir);
}
int main(int argc, char **argv)
{
	if (argIs("/D"))
	{
		char *rDir;
		char *wDir;

		cout("+------------------------------+\n");
		cout("| �R�s�[����Œ肵�ĕ����R�s�[ |\n");
		cout("+------------------------------+\n");

		if (hasArgs(1))
		{
			wDir = strx(nextArg());
		}
		else
		{
			cout("�Œ肳�ꂽ�R�s�[��f�B���N�g��(���[�g):\n");
			wDir = dropDir();
		}

		for (; ; )
		{
			cout("�R�s�[���f�B���N�g��:\n");
			rDir = dropDir();

			DoCopy(rDir, wDir);

			memFree(rDir);

			cout("\n");
		}
		return; // dummy
	}

	if (hasArgs(2))
	{
		DoCopy(getArg(0), getArg(1));
		return;
	}

	for (; ; )
	{
		char *rDir;
		char *wDir;

		cout("�R�s�[���f�B���N�g��:\n");
		rDir = dropDir();
		cout("�R�s�[��f�B���N�g��(���[�g):\n");
		wDir = dropDir();

		DoCopy(rDir, wDir);

		memFree(rDir);
		memFree(wDir);

		cout("\n");
	}
}
