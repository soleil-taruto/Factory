#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	if (!strcmp("1", getEnvLine("@NoPause")))
	{
		LOGPOS();
		return;
	}
	cout("���s����ɂ͉����L�[�������ĉ����� . . .\n");
	clearGetKey();
}
