/*
	EndProc �ɂ���ăG���[�_�C�A���O��\������O�� exit() ����B
	�Ȃ��AEndProc ���� termination() �ɂ���� Finalizers �̑��������s�����̂ŁAEcho6�`1 ���ׂĎ��s�����B
*/

#include "C:\Factory\Common\all.h"

#define Echo(num) \
	static void Echo##num(void) { cout("%u\n", num); }

Echo(1)
Echo(2)
Echo(3)
Echo(4)
Echo(5)
Echo(6)

static void EndProc(void)
{
	LOGPOS();
	termination(1);
}
int main(int argc, char **argv)
{
	addFinalizer(Echo1);
	addFinalizer(Echo2);
	addFinalizer(Echo3);

	addFinalizer(EndProc);

	addFinalizer(Echo4);
	addFinalizer(Echo5);
	addFinalizer(Echo6);

	error();
}
