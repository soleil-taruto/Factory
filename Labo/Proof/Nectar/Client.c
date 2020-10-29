/*
	START Server.exe

	で、起動しておいて、

	Client.exe 朝比奈みらいちゃんのなんたらかんたら...

	ってやる。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Nectar.h"

int main(int argc, char **argv)
{
	Nectar_t *i = CreateNectar("Nectar_Test");

	NectarSendLine(i, nextArg());
	ReleaseNectar(i);
}
