/*
	START Server.exe

	�ŁA�N�����Ă����āA

	Client.exe ���R�������Ȃ񂽂炩�񂽂�...

	���Ă��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Nectar2.h"

int main(int argc, char **argv)
{
	Nectar2_t *i = CreateNectar2("Kira_Kira_PRE-CURE_a_la_mode");

	Nectar2SendLine(i, nextArg());
	Nectar2SendChar(i, 0x00); // �f���~�^�𑗂�B

	ReleaseNectar2(i);
}
