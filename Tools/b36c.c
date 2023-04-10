/*
	b36c.exe [/L] [/P]

		/L ... 小文字
		/P ... 表示のみ。(エディタを開かない)

	----

	b36c == Base-36 C(100) chars

	36 P 100 L 2 == 516.*

	----
	書式

	{B36C-XXXXXXXXXXXXXXXXXXXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXX-XXXXXXXXXXXXXXXXXXXXXXXXX}
	      ~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~
	               25文字                    25文字                    25文字                    25文字

		X ... Base-36 char
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

#define BASE36_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

static char *MakeB36C(void)
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
	char *b36c = MakeB36C();

	if (argIs("/L"))
		toLowerLine(b36c);

	cout("%s\n", b36c);

	if (!argIs("/P")) // ? not Print only
		viewLine(b36c);

	memFree(b36c);
}
