/*
	b26110.exe [/L] [/P]

		/L ... ������
		/P ... �\���̂݁B(�G�f�B�^���J���Ȃ�)

	----

	b26110 == Base-26(A-Z) 110 chars

	26 P 110 L 2 == 517.*

	----
	����

	{AZCX-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY}
	      ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~
	              22����                 22����                 22����                 22����                 22����
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	                                                         121����

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
