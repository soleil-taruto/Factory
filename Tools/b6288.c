/*
	b6288.exe [/P]

		/P ... 表示のみ。(エディタを開かない)

	----

	b6288 == Base-62 88 chars

	62 P 88 L 2 == 524.*

	----
	書式

	{b6288-XXXXXXXXXXXXXXXXXXXXXX-XXXXXXXXXXXXXXXXXXXXXX-XXXXXXXXXXXXXXXXXXXXXX-XXXXXXXXXXXXXXXXXXXXXX}
	       ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~
	               22文字                 22文字                 22文字                 22文字

		X ... Base-62 char
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

#define BASE62_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

static char *MakeB6288(void)
{
	char *buff = strx("");
	uint index;

	buff = addLine(buff, "{b6288-");

	for (index = 0; index < 88; index++)
	{
		if (index && index % 22 == 0)
			buff = addChar(buff, '-');

		buff = addChar(buff, BASE62_CHARS[(uint)getCryptoRand64Mod(62)]);
	}
	buff = addChar(buff, '}');
	return buff;
}
int main(int argc, char **argv)
{
	char *b6288 = MakeB6288();

	cout("%s\n", b6288);

	if (!argIs("/P")) // ? not Print only
		viewLine(b6288);

	memFree(b6288);
}
