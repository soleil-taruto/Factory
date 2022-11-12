#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

int main(int argc, char **argv)
{
	char *dir = "C:\\temp";
	uint filecnt = 10000;
	int dataflag = 0;
	uint count;

readArgs:
	if (argIs("/O"))
	{
		dir = nextArg();
		goto readArgs;
	}
	if (argIs("/C"))
	{
		filecnt = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/D"))
	{
		dataflag = 1;
		goto readArgs;
	}

	if (dataflag)
	{
		for (count = 0; count < filecnt; count++)
		{
			writeOneLineNoRet_b_xx(combine_cx(dir, MakeUUID(1)), MakeUUID(1));
		}
	}
	else
	{
		for (count = 0; count < filecnt; count++)
		{
			createFile_x(combine_cx(dir, MakeUUID(1)));
		}
	}
}
