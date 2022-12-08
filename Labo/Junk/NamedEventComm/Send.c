/*
	Send.exe [/F TEXT-FILE | MESSAGE]

	- - -

	使い方

		1. START Recv.exe とかやって実行しておく。

		2. Send.exe ABCDEF とか実行する。

		Recv.exe の方に ABCDEF とか出たら成功！
*/

#include "C:\Factory\Common\all.h"
#include "Define.h"

static void DoSendBit(uint bit)
{
	uint evReady = eventOpen(EV_READY);
	uint evBit_0 = eventOpen(EV_BIT_0);
	uint evBit_1 = eventOpen(EV_BIT_1);
	int result;

	if (handleWaitForMillis(evReady, 5000))
	{
		if (bit)
			eventSet(evBit_1);
		else
			eventSet(evBit_0);

		result = 1;
	}
	else
	{
		result = 0;
	}

	handleClose(evReady);
	handleClose(evBit_0);
	handleClose(evBit_1);

	errorCase_m(!result, "送信タイムアウトしました。受信側の準備ができていません。");
}
static void DoSend(char *message)
{
	char *p;

	LOGPOS();

	for (p = message; *p; p++)
	{
		uint bit;

		for (bit = 1 << 7; bit; bit >>= 1)
		{
			DoSendBit((uint)*p & bit);
		}
	}
	LOGPOS();
}
int main(int argc, char **argv)
{
	if (argIs("/F"))
	{
		char *text = readText(nextArg());

		DoSend(text);
		memFree(text);
		return;
	}
	if (hasArgs(1))
	{
		DoSend(nextArg());
		return;
	}

	for (; ; )
	{
		char *text = readText(c_dropFile());

		DoSend(text);
		memFree(text);
		cout("\n");
	}
}
