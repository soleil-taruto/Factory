/*
	オプション省略 -> 自分のIP

	- - - -

	ファイアウォールを有効にしていると gethosybyname() で IP が取れない。
	「ファイルとプリンタの共有」を例外にすると取得できるようになる。
	ping に応答するかしないかが関係？
*/

#include "C:\Factory\Common\Options\SockClient.h"

int main(int argc, char **argv)
{
	autoList_t *ips = sockLookupList(hasArgs(1) ? nextArg() : "");
	uchar ip[4];
	uint index;

	foreach (ips, ip, index)
	{
		cout("%s\n", SockIp2Line(ip));
	}
	releaseAutoList(ips);
}
