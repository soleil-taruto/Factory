/*
	�א؂�̃������u���b�N���ʂɊm�ۂ��Ď��s���Ă��� system �R�[�����Ă��R�}���h�����s����Ȃ��B@ 2015.7.23

	�������l

		memAllocErrorAndExecute 2949088 -> DIR ���s����Ȃ��B
		memAllocErrorAndExecute 2949089 -> DIR ���s�����B

	-> 3MB���炢�H
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	uint size = toValue(nextArg());

	LOGPOS();

	while (malloc(size));

	LOGPOS();

	execute("DIR");

	LOGPOS();
}
