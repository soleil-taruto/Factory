#include "C:\Factory\Common\all.h"

static void Test_lineToFairLocalPath(char *str, uint sz)
{
	char *ret;

	cout("< %s\n", str);
	cout("< %u\n", sz);

	ret = lineToFairLocalPath(str, sz);

	cout("> %s\n", ret);

	// çÏê¨ÉeÉXÉg
	{
		addCwd("C:\\temp");
		createFile(ret);
		errorCase(!existFile(ret));
		unaddCwd();
	}
	memFree(ret);
}
int main(int argc, char **argv)
{
	if (argIs("/L2FLP"))
	{
		if (hasArgs(2))
		{
			Test_lineToFairLocalPath(getArg(0), toValue(getArg(1)));
		}
		else
		{
			Test_lineToFairLocalPath(getArg(0), 0);
		}
		return;
	}
}
