/*
	Send.exe [/F TEXT-FILE | MESSAGE]

	- - -

	使い方

		1. START Recv.exe とかやって実行しておく。

		2. Send.exe ABCDEF とか実行する。

		Recv.exe の方に ABCDEF とか出たら成功！
*/

#include "C:\Factory\Common\all.h"
#include "Common.h"

static void DoSendBit(uint b0, uint b1)
{
	static uint m0;
	uint m1;

	m0++;
	m1 = m0 + 1;
	m0 %= 3;
	m1 %= 3;

	M_Set(M_MUTEX_0 + m1, 1);
	M_Set(M_MUTEX_0 + m0, 0);
	M_Set(M_BIT_0_0 + m1, b0);
	M_Set(M_BIT_1_0 + m1, b1);
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
			if (*p & bit)
				DoSendBit(0, 1);
			else
				DoSendBit(1, 0);
		}
	}
	DoSendBit(0, 0);
	DoSendBit(0, 0);
	DoSendBit(0, 0);

	/*
		Recv.exe の M_Set() 待ちを回避するため、、
	*/
	M_Set(M_MUTEX_0, 0);
	M_Set(M_MUTEX_1, 0);
	M_Set(M_MUTEX_2, 0);

	LOGPOS();
}
static void Main2(void)
{
	M_Init();
	addFinalizer(M_Fnlz);

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
int main(int argc, char **argv)
{
	Main2();
	termination(0);
}
