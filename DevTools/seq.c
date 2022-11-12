/*
	seq.exe [/C WIDTH START-VAL COUNT STEP-VAL | /P PLAIN-PTN | PLAIN-PTN]...

	- - -
	ó·:

	seq.exe "Ç±ÇÃçsÇÕ " /C 1 1 100 1 " çsñ⁄Ç≈Ç∑ÅB"
*/

#include "C:\Factory\Common\all.h"

static void PrintSq(char *line)
{
	uint back_ai = getArgIndex();

	if (!hasArgs(1))
	{
		cout("%s\n", line);
	}
	else if (argIs("/C"))
	{
		uint width;
		uint startVal;
		uint count;
		uint stepVal;
		uint index;

		width    = toValue(nextArg());
		startVal = toValue(nextArg());
		count    = toValue(nextArg());
		stepVal  = toValue(nextArg());

		for (index = 0; index < count; index++)
		{
			uint value = startVal + index * stepVal;
			char *sVal;
			char *buff;

			sVal = xcout("%u", value);

			while (strlen(sVal) < width)
				sVal = insertChar(sVal, 0, '0');

			buff = strx(line);
			buff = addLine(buff, sVal);

			PrintSq(buff);

			memFree(buff);
			memFree(sVal);
		}
	}
	else
	{
		char *buff;

		argIs("/P");

		buff = strx(line);
		buff = addLine(buff, nextArg());

		PrintSq(buff);

		memFree(buff);
	}
	setArgIndex(back_ai);
}
int main(int argc, char **argv)
{
	PrintSq("");
}
