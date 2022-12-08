// smpl
/*
	�f�B���N�g���̃R�s�[

	----

	xcp.exe [/C] [/V [/R]] SOURCE-DIR DESTINATION-DIR

	SOURCE-DIR
		�R�s�[���f�B���N�g��
			�A�� /V �w�莞�͈ړ����f�B���N�g��
		���݂���f�B���N�g���ł��邱�ƁB
		�ύX���Ȃ��B
			�A�� /V �w�莞�͋�̃f�B���N�g���ɂȂ�B
				�A�� /R �w�莞�͍폜�����B

	DESTINATION-DIR
		�R�s�[��f�B���N�g��
			�A�� /V �w�莞�͈ړ���f�B���N�g��
		���݂����̃f�B���N�g���ł��邱�ƁB(��łȂ��ꍇ�͖��O�����Ȃ�����)
			�A�� /C �w�莞�͑��݂��Ȃ��쐬�\�ȃp�X�ł��邱�ƁB
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *srcDir;
	char *destDir;
	int create_mode = 0;
	int move_mode = 0;
	int remove_mode = 0;

readArgs:
	if (argIs("/C") || argIs("/MD"))
	{
		create_mode = 1;
		goto readArgs;
	}
	if (argIs("/V") || argIs("/MV"))
	{
		move_mode = 1;
		goto readArgs;
	}
	if (argIs("/R") || argIs("/RM"))
	{
		remove_mode = 1;
		goto readArgs;
	}

	srcDir  = nextArg();
	destDir = nextArg();

	if (create_mode)
		createDir(destDir);

	if (move_mode)
	{
		moveDir(srcDir, destDir);

		if (remove_mode)
			removeDir(srcDir);
	}
	else
	{
		copyDir(srcDir, destDir);
	}
}
