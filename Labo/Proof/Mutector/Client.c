/*
	START Server.exe

	�ŁA�N�����Ă����āA

	Client.exe ���Y�搶�̂Ȃ񂽂炩�񂽂�...

	���Ă��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Mutector.h"

int main(int argc, char **argv)
{
	Mutector_t *i = CreateMutector("Mutector_Test");

	MutectorSendLine(i, nextArg());
	ReleaseMutector(i);
}
