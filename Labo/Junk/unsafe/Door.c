/*
	���C���̃m�[�g�Ŏ�������g���C���������B
*/

#include "C:\Factory\Common\all.h"

#pragma comment(lib, "WinMM.lib")

#define DoorOpen() \
	mciSendString("Set CDaudio Door Open", NULL, 0, NULL)

#define DoorClose() \
	mciSendString("Set CDaudio Door Closed", NULL, 0, NULL)

int main(int argc, char **argv)
{
	if (argIs("/O")) // Open
	{
		DoorOpen();
	}
	else if (argIs("/C")) // Close
	{
		DoorClose();
	}
	else
	{
		DoorOpen();

		cout("�G�X�P�[�v�L�[�ȊO�������ƃg���C����܂��B\n");

		if (clearGetKey() != 0x1b)
			DoorClose();
	}
}
