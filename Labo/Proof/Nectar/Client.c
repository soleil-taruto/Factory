/*
	START Server.exe

	�ŁA�N�����Ă����āA

	Client.exe ����ނ݂炢�����̂Ȃ񂽂炩�񂽂�...

	���Ă��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Nectar.h"

int main(int argc, char **argv)
{
	Nectar_t *i = CreateNectar("Nectar_Test");

	NectarSendLine(i, nextArg());
	ReleaseNectar(i);
}
