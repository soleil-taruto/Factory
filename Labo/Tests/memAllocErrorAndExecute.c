/*
	細切れのメモリブロックを大量に確保して失敗してから system コールしてもコマンドが実行されない。@ 2015.7.23

	しきい値

		memAllocErrorAndExecute 2949088 -> DIR 実行されない。
		memAllocErrorAndExecute 2949089 -> DIR 実行される。

	-> 3MBくらい？
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	uint size = toValue(nextArg());

	LOGPOS();

	while (malloc(size));

	LOGPOS();

	execute("DIR");

	LOGPOS();
}
