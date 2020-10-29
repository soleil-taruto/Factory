/*
	START Server.exe

	で、起動しておいて、

	Client.exe リコちゃんをなんたらかんたら...

	ってやる。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Nectar2.h"

int main(int argc, char **argv)
{
	Nectar2_t *i = CreateNectar2("Kira_Kira_PRE-CURE_a_la_mode");

	Nectar2SendLine(i, nextArg());
	Nectar2SendChar(i, 0x00); // デリミタを送る。

	ReleaseNectar2(i);
}
