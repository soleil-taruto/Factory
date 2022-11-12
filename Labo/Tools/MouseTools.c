#pragma comment(lib, "user32.lib")

#include "C:\Factory\Common\all.h"

static void DoClick(uint x, uint y, int leftFlag)
{
	SetCursorPos(x, y);
	sleep(10);
	mouse_event(leftFlag ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
	sleep(30);
	mouse_event(leftFlag ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	sleep(10);
}
int main(int argc, char **argv)
{
	ClipCursor(NULL); // âÊñ è„Ç«Ç±Ç≈Ç‡çsÇØÇÈÇÊÇ§Ç…Ç∑ÇÈÅB

readArgs:
	if (argIs("/P"))
	{
		uint x = toValue(nextArg());
		uint y = toValue(nextArg());

		SetCursorPos(x, y);

		goto readArgs;
	}
	if (argIs("/LC"))
	{
		uint x = toValue(nextArg());
		uint y = toValue(nextArg());

		DoClick(x, y, 1);

		goto readArgs;
	}
	if (argIs("/RC"))
	{
		uint x = toValue(nextArg());
		uint y = toValue(nextArg());

		DoClick(x, y, 0);

		goto readArgs;
	}
	if (argIs("/LD"))
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		goto readArgs;
	}
	if (argIs("/RD"))
	{
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
		goto readArgs;
	}
	if (argIs("/LU"))
	{
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		goto readArgs;
	}
	if (argIs("/RU"))
	{
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
		goto readArgs;
	}
	if (argIs("/W"))
	{
		sleep(toValue(nextArg()));
		goto readArgs;
	}
}
