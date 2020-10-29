#pragma once

#include "C:\Factory\Common\all.h"

typedef struct VTree_st
{
	uint (*GetLocalCount)(void);   // カレントディレクトリのディレクトリ・ファイル総数を返す。
	char *(*GetLocal)(uint index); // ローカルファイル名を返す。★★★戻り値は呼び出し側で開放すること★★★
	int (*IsDir)(uint index);      // ディレクトリであるかどうか返す。
	void (*IntoDir)(uint index);   // ディレクトリに入る。
	void (*ExitDir)(void);         // 親ディレクトリへ移動する。
	uint64 (*GetSize)(uint index); // ファイルのサイズを返す。
	void (*GetEntity)(uint index, uint64 startPos, uint size, void *outBuff); // ファイルの部分を返す。
}
VTree_t;
