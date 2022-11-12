/*
	[ドラッグ]

	MouseCtrl.exe /P 100 100 /LD /P 200 200  <--  これで 100, 100 から 200, 200 まで左ボタンを下げたままドラッグしたことになる。

	[左クリック]

	MouseCtrl.exe /P 500 400 /LD /W 100 /LU  <--  押し下げ時間をある程度取らないとクリックできないことがあるようだ。
*/

#pragma comment(lib, "user32.lib")

#include "C:\Factory\Common\all.h"

#define SCREEN_X_MIN -10000
#define SCREEN_Y_MIN -10000
#define SCREEN_X_MAX 20000
#define SCREEN_Y_MAX 20000

int main(int argc, char **argv)
{
	ClipCursor(NULL); // 画面上どこでも行けるようにする。

argsLoop:
	if (argIs("/P")) // マウス移動
	{
		int x;
		int y;

		x = atoi(nextArg());
		y = atoi(nextArg());

		errorCase(!m_isRange(x, SCREEN_X_MIN, SCREEN_X_MAX));
		errorCase(!m_isRange(y, SCREEN_Y_MIN, SCREEN_Y_MAX));

		SetCursorPos(x, y);

		goto argsLoop;
	}
	if (argIs("/LD")) // 左ボタン下げ
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		goto argsLoop;
	}
	if (argIs("/LU")) // 左ボタン上げ
	{
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		goto argsLoop;
	}
	if (argIs("/RD")) // 右ボタン下げ
	{
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
		goto argsLoop;
	}
	if (argIs("/RU")) // 右ボタン上げ
	{
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
		goto argsLoop;
	}
	if (argIs("/W")) // ウェイト
	{
		int slpcnt = toValue(nextArg());
		errorCase(!m_isRange(slpcnt, 0, 60000)); // 0 ～ 1分
		sleep(slpcnt);

		goto argsLoop;
	}

	// ----

	if (argIs("/FLC")) // フラッシュ？左クリック
	{
		int x;
		int y;
		int slpcnt;

		x = toValue(nextArg());
		y = toValue(nextArg());
		slpcnt = toValue(nextArg());

		SetCursorPos(x, y);
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		sleep(100);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		coSleep(slpcnt);

		goto argsLoop;
	}
}
