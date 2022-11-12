/*
	namedTrack.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	namedTrack.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] TRACK-NAME

		CONNECT-MAX ... 最大接続数, 省略時は 1000
		TRACK-NAME ... トラック名

	----
	送信データ

		TRACK-NAME + 0x0d + 0x0a + SEND-DATA

			TRACK-NAME: 1*100( C-SYMBOL + '-' + '.' + ':' )
			SEND-DATA: *( 0x00 ... 0xff )
*/

#include "libs\Tunnel.h"

#define TRACKNAME_LENMAX 100

static char *TrackName;

static void NamedChannelFltr(autoBlock_t *buff, uint p_nsFlag)
{
	if (!*(int *)p_nsFlag)
	{
		autoBlock_t *nb = newBlock();

		ab_addLine(nb, TrackName);
		ab_addLine(nb, "\r\n");
		ab_addBytes(nb, buff);

		ab_swap(nb, buff);

		releaseAutoBlock(nb);

		*(int *)p_nsFlag = 1;
	}
}
static void Perform(int sock, int fwdSock)
{
	int nsFlag = 0;

	CrossChannel(sock, fwdSock, NamedChannelFltr, (uint)&nsFlag, NULL, 0);
}
static int ReadArgs(void)
{
	TrackName = strx(nextArg());
	line2csym_ext(TrackName, "-.:");

	errorCase(m_isEmpty(TrackName) || TRACKNAME_LENMAX < strlen(TrackName));

	return 0;
}
static char *GetTitleSuffix(void)
{
	return xcout("N:%s", TrackName);
}
int main(int argc, char **argv)
{
	TunnelMain(ReadArgs, Perform, "namedTrack", GetTitleSuffix);
}
