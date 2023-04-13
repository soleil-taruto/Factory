/*
	b10155.exe [/L] [/P]

		/L ... ������
		/P ... �\���̂݁B(�G�f�B�^���J���Ȃ�)

	----

	b10155 == Base-10(10�i��) 155 chars

	10 P 155 L 2 == 514.*

	----
	����

	{10155-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY}
	       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	                   31����                          31����                          31����                          31����                          31����
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	                                                                                167����

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
