/*
	Stopwatch.exe �R�}���h���C��
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

	cout("���s���� %I64u �~���b\n", ed - st);
}
