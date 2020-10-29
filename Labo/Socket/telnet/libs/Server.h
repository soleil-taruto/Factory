#pragma once

#include "C:\Factory\Common\Options\SockServer.h"

uint CreateTelnetServerPerformInfo(void); // external
void ReleaseTelnetServerPerformInfo(uint); // external

/*
	�ڑ������x���Ă΂��B

	inputLine:
		NULL == ���͍s�����B
		NULL != ���͍s, toAsciiiLine(, 0, 0, 1) �ς�, �Ăяo�����ŊJ������̂� TelnetServerPerform ���ŊJ�����Ă͂Ȃ�Ȃ��B

	prm:
		CreateTelnetServerPerformInfo() �̖߂�l

	ret:
		NULL == �ؒf
		NULL != �o�̓e�L�X�g, ���s�� CR-LF �ł��邱�ƁB�Ăяo�����ŊJ������̂� TelnetServerPerform ���Œ͂�ł����� const �ȕ�����ł����Ă͂Ȃ�Ȃ��B
*/
char *TelnetServerPerform(char *inputLine, uint prm); // external
