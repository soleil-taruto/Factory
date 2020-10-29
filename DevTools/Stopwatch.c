/*
	Stopwatch.exe コマンドライン
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *cmdln = untokenize(allArgs(), " ");
	uint64 st;
	uint64 ed;

	st = nowTick();
	system(cmdln);
	ed = nowTick();

	cout("実行時間 %I64u ミリ秒\n", ed - st);
}
