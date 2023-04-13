/*
	b36100.exe [/L] [/P]

		/L ... 小文字
		/P ... 表示のみ。(エディタを開かない)

	----

	b36100 == Base-36 100 chars

	36 P 100 L 2 == 516.*

	----
	書式

	{B36C-YYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYY}
	      ~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~
	               25文字                    25文字                    25文字                    25文字
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	                                                   110文字

		Y ... Base-36 char

		B36C ... Base-36 100(C) chars
		         ^    ^^     ^
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

#define BASE36_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

static char *MakeB36100(void)
{
	char *buff = strx("");
	uint index;

	buff = addLine(buff, "{B36C-");

	for (index = 0; index < 100; index++)
	{
		if (index && index % 25 == 0)
			buff = addChar(buff, '-');

		buff = addChar(buff, BASE36_CHARS[(uint)getCryptoRand64Mod(36)]);
	}
	buff = addChar(buff, '}');
	return buff;
}
int main(int argc, char **argv)
{
	char *b36100 = MakeB36100();

	if (argIs("/L"))
		toLowerLine(b36100);

	cout("%s\n", b36100);

	if (!argIs("/P")) // ? not Print only
		viewLine(b36100);

	memFree(b36100);
}
