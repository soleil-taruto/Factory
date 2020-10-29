#pragma once

#include "C:\Factory\Common\Options\SockServer.h"

uint CreateTelnetServerPerformInfo(void); // external
void ReleaseTelnetServerPerformInfo(uint); // external

/*
	接続中何度も呼ばれる。

	inputLine:
		NULL == 入力行無し。
		NULL != 入力行, toAsciiiLine(, 0, 0, 1) 済み, 呼び出し側で開放するので TelnetServerPerform 内で開放してはならない。

	prm:
		CreateTelnetServerPerformInfo() の戻り値

	ret:
		NULL == 切断
		NULL != 出力テキスト, 改行は CR-LF であること。呼び出し側で開放するので TelnetServerPerform 内で掴んでいたり const な文字列であってはならない。
*/
char *TelnetServerPerform(char *inputLine, uint prm); // external
