/*
	b26110.exe [/L] [/P]

		/L ... 小文字
		/P ... 表示のみ。(エディタを開かない)

	----

	b26110 == Base-26(A-Z) 110 chars

	26 P 110 L 2 == 517.*

	----
	書式

	{AZCX-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY}
	      ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~
	              22文字                 22文字                 22文字                 22文字                 22文字
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	                                                         121文字

		Y ... Base-26 char

		AZCX ... [A-Z] 110(CX) chars
		          ^ ^      ^^
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

#define BASE26_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

static char *MakeB26110(void)
{
	char *buff = strx("");
	uint index;

	buff = addLine(buff, "{AZCX-");

	for (index = 0; index < 110; index++)
	{
		if (index && index % 22 == 0)
			buff = addChar(buff, '-');

		buff = addChar(buff, BASE26_CHARS[(uint)getCryptoRand64Mod(26)]);
	}
	buff = addChar(buff, '}');
	return buff;
}
int main(int argc, char **argv)
{
	char *b26110 = MakeB26110();

	if (argIs("/L"))
		toLowerLine(b26110);

	cout("%s\n", b26110);

	if (!argIs("/P")) // ? not Print only
		viewLine(b26110);

	memFree(b26110);
}
