// smpl
/*
	TextMerge.exe [/S PART-SIZE] [/S1] [/S2] [/I] INPUT-TEXT-FILE-1 INPUT-TEXT-FILE-2
	              (OUTPUT-LEFT-ONLY-TEXT-FILE | *) (OUTPUT-BOTH-EXIST-TEXT-FILE | *) (OUTPUT-RIGHT-ONLY-TEXT-FILE | *)
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\MergeSort.h"

static char *ArgPathFltr(char *arg)
{
	if (!strcmp(arg, "*"))
		return NULL;

	return arg;
}
int main(int argc, char **argv)
{
	uint partSize = 128 * 1024 * 1024;
	int sorted1 = 0;
	int sorted2 = 0;

readArgs:
	if (argIs("/S"))
	{
		partSize = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/S1"))
	{
		sorted1 = 1;
		goto readArgs;
	}
	if (argIs("/S2"))
	{
		sorted2 = 1;
		goto readArgs;
	}

	if (argIs("/I"))
	{
		MergeFileTextICase(getArg(0), sorted1, getArg(1), sorted2, ArgPathFltr(getArg(2)), ArgPathFltr(getArg(3)), ArgPathFltr(getArg(4)), partSize);
	}
	else
	{
		MergeFileText(getArg(0), sorted1, getArg(1), sorted2, ArgPathFltr(getArg(2)), ArgPathFltr(getArg(3)), ArgPathFltr(getArg(4)), partSize);
	}
}
