/*
	START Server.exe

	で、起動しておいて、

	Client.exe リズ先生のなんたらかんたら...

	ってやる。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Mutector.h"

int main(int argc, char **argv)
{
	Mutector_t *i = CreateMutector("Mutector_Test");

	MutectorSendLine(i, nextArg());
	ReleaseMutector(i);
}
