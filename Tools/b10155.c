/*
	b10155.exe [/L] [/P]

		/L ... 小文字
		/P ... 表示のみ。(エディタを開かない)

	----

	b10155 == Base-10(10進数) 155 chars

	10 P 155 L 2 == 514.*

	----
	書式

	{10155-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY}
	       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	                   31文字                          31文字                          31文字                          31文字                          31文字
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	                                                                                167文字

		Y ... Hex(Base-16) char

		10155 ... Base-10 115 chars
		               ^^ ^^^
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

#define BASE10_CHARS "0123456789"

static char *MakeB10155(void)
{
	char *buff = strx("");
	uint index;

	buff = addLine(buff, "{10155-");

	for (index = 0; index < 155; index++)
	{
		if (index && index % 31 == 0)
			buff = addChar(buff, '-');

		buff = addChar(buff, BASE10_CHARS[(uint)getCryptoRand64Mod(10)]);
	}
	buff = addChar(buff, '}');
	return buff;
}
int main(int argc, char **argv)
{
	char *b10155 = MakeB10155();

	if (argIs("/L"))
		toLowerLine(b10155);

	cout("%s\n", b10155);

	if (!argIs("/P")) // ? not Print only
		viewLine(b10155);

	memFree(b10155);
}
