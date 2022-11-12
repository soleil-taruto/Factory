/*
	wait.exe [/S] [/ANTSS] [-7 | -37 | -0000 | -3000 | -090000 �␳�b�� | �҂b��]

	----
	���s��

	wait.exe -090000 0       ... ���� 09:00:00 ��҂B
	wait.exe -090000 1800    ... ���� 09:30:00 ��҂B
	wait.exe -090000 3600    ... ���� 10:00:00 ��҂B
	wait.exe -090000 10800   ... ���� 12:00:00 ��҂B
	wait.exe -090000 18000   ... ���� 14:00:00 ��҂B
	wait.exe -090000 32400   ... ���� 18:00:00 ��҂B
	wait.exe -090000 33333   ... ���� 18:15:33 ��҂B
	wait.exe -090000 36000   ... ���� 19:00:00 ��҂B
	wait.exe -090000 54000   ... ���� 00:00:00 ��҂B

	----
	�I���R�[�h (�G���[���x��)

	�^�C���A�E�g�����ꍇ     ... 0
	�G���^�[���������ꍇ     ... 0
	�G�X�P�[�v���������ꍇ   ... 1

	----
	�L�[����

	+            ... �c�莞�� += 1��
	-            ... �c�莞�� -= 1�� (�A���A�c�莞�� 5�b �����ɂȂ�Ȃ��悤�ɂ���)
	*            ... �c�莞�� += 1����
	/            ... �c�莞�� = 5�b
	�G���^�[     ... ���^�C���A�E�g
	�G�X�P�[�v   ... ���^�C���A�E�g
*/

//#pragma comment(lib, "user32.lib") // for mouse

#include "C:\Factory\Common\all.h"

static void DoAntiScreenSaver(void)
{
#if 1
	SetThreadExecutionState(ES_SYSTEM_REQUIRED);
	SetThreadExecutionState(ES_DISPLAY_REQUIRED);
#else
	SetCursorPos(0, 0); // ��ʍ���
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	sleep(100);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
#endif
}
int main(int argc, char **argv)
{
	int simpleMode = 0;
	int antiScreenSaver = 0;
	uint errorLevel = 0;
	uint remain;

readArgs:
	if (argIs("/S"))
	{
		simpleMode = 1;
		goto readArgs;
	}
	if (argIs("/ANTSS"))
	{
		cout("+--------+\n");
		cout("| ANT-SS |\n");
		cout("+--------+\n");

		antiScreenSaver = 1;
		goto readArgs;
	}

	if (argIs("-7")) // ���� 7, 17, 27, 37, 47, 57 �b��҂B
	{
		cout("next x7 second\n");
		remain = (17 - (uint)(time(NULL) % 10i64)) % 10;
		cout("remain: %u\n", remain);
	}
	else if (argIs("-37")) // ���� 37 �b��҂B
	{
		cout("next 37 second\n");
		remain = (97 - (uint)(time(NULL) % 60i64)) % 60;
		cout("remain: %u\n", remain);
	}
	else if (argIs("-0000")) // ���� hh:00:00 ��҂B
	{
		cout("next hh:00:00\n");
		remain = 3600 - (uint)(time(NULL) % 3600i64);
		cout("remain: %u\n", remain);
	}
	else if (argIs("-3000")) // ���� hh:00:00 or hh:30:00 ��҂B
	{
		cout("next hh:00:00 or hh:30:00\n");
		remain = 1800 - (uint)(time(NULL) % 1800i64);
		cout("remain: %u\n", remain);
	}
	else if (argIs("-090000")) // ���� HH:MM:SS ��҂B
	{
		uint prmSec = toValue(nextArg()) % 86400;

		cout("next 09:00:00 + %u sec\n", prmSec);
		remain = 86400 - (uint)((time(NULL) - prmSec) % 86400i64);
		cout("remain: %u\n", remain);
	}
	else
	{
		remain = toValue(nextArg());
	}

	errorCase_m(hasArgs(1), "�s���ȃR�}���h����");

	for (; ; )
	{
		if (simpleMode)
			cout("\r%u ", remain);
		else
			cout("\rwait for ESCAPE or other keys while %u seconds... ", remain);

		while (hasKey())
		{
			switch (getKey())
			{
			case 0x1b:
				errorLevel = 1;
				goto endLoop;

			case 0x0d:
				goto endLoop;

			case '+':
				remain += 60;
				break;

			case '-':
				if (remain < 65)
					remain = 5;
				else
					remain -= 60;

				break;

			case '*':
				remain += 3600;
				break;

			case '/':
				if (remain < 3605)
					remain = 5;
				else
					remain -= 3600;

				break;
			}
		}
		if (!remain)
			break;

		if (antiScreenSaver && remain % 30 == 0)
		{
			DoAntiScreenSaver();
		}
		sleep(1000);
		remain--;
	}
endLoop:
	cout("\n%u\n", errorLevel);
	termination(errorLevel);
}
