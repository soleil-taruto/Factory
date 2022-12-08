#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Calc2.h"

int main(int argc, char **argv)
{
	char *numb = repeatChar('9', hasArgs(1) ? toValue(nextArg()) : 50);
//	char *numb = repeatChar('9', 50);

	for (; ; )
	{
		cout("%s\n", numb);

		if (!strcmp(numb, "1"))
			break;

		if (strchr(numb, '\0')[-1] & 1) // ? äÔêî
		{
			numb = calc_xc(numb, '*', "3");
			numb = calc_xc(numb, '+', "1");
		}
		else
		{
//			numb = calc_xc(numb, '/', "2");
			numb = calc_xc(numb, '*', "0.5");
		}
	}
	memFree(numb);
	termination(0); // mem_dbg
}
