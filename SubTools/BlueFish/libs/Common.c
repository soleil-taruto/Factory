#include "Common.h"

static int Real_IsBlueFishComputer(void)
{
	return existDir("C:\\BlueFish");
}
int IsBlueFishComputer(void)
{
	static int inited;
	static int ret;

	if(!inited)
	{
		ret = Real_IsBlueFishComputer();
		cout("この環境は BlueFish で%s\n", ret ? "す。" : "はありません。");
		inited = 1;
	}
	return ret;
}
