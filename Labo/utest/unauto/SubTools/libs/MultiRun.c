#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\MultiRun.h"

static void DoTest_01(void)
{
	autoList_t *emptyList = newList();

	MultiRun_Commands(emptyList); // ���X�g��ł����s�ł���͂��B

	releaseAutoList(emptyList);
}
int main(int argc, char **argv)
{
	DoTest_01();
}
