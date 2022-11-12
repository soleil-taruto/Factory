/*
	[�h���b�O]

	MouseCtrl.exe /P 100 100 /LD /P 200 200  <--  ����� 100, 100 ���� 200, 200 �܂ō��{�^�����������܂܃h���b�O�������ƂɂȂ�B

	[���N���b�N]

	MouseCtrl.exe /P 500 400 /LD /W 100 /LU  <--  �����������Ԃ�������x���Ȃ��ƃN���b�N�ł��Ȃ����Ƃ�����悤���B
*/

#pragma comment(lib, "user32.lib")

#include "C:\Factory\Common\all.h"

#define SCREEN_X_MIN -10000
#define SCREEN_Y_MIN -10000
#define SCREEN_X_MAX 20000
#define SCREEN_Y_MAX 20000

int main(int argc, char **argv)
{
	ClipCursor(NULL); // ��ʏ�ǂ��ł��s����悤�ɂ���B

argsLoop:
	if (argIs("/P")) // �}�E�X�ړ�
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
	if (argIs("/LD")) // ���{�^������
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		goto argsLoop;
	}
	if (argIs("/LU")) // ���{�^���グ
	{
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		goto argsLoop;
	}
	if (argIs("/RD")) // �E�{�^������
	{
		mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
		goto argsLoop;
	}
	if (argIs("/RU")) // �E�{�^���グ
	{
		mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
		goto argsLoop;
	}
	if (argIs("/W")) // �E�F�C�g
	{
		int slpcnt = toValue(nextArg());
		errorCase(!m_isRange(slpcnt, 0, 60000)); // 0 �` 1��
		sleep(slpcnt);

		goto argsLoop;
	}

	// ----

	if (argIs("/FLC")) // �t���b�V���H���N���b�N
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
