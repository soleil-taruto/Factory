/*
	b6288.exe [/P]

		/P ... �\���̂݁B(�G�f�B�^���J���Ȃ�)

	----

	b6288 == Base-62 88 chars

	62 P 88 L 2 == 524.*

	----
	����

	{B62ee-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY-YYYYYYYYYYYYYYYYYYYYYY}
	       ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~
	               22����                 22����                 22����                 22����
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	                                              98����

		Y ... Base-62 char

		B62ee ... Base-62 eighty-eight(88) chars
		          ^    ^^ ^      ^
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

#define BASE62_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

static char *MakeB6288(void)
{
	char *buff = strx("");
	uint index;

	buff = addLine(buff, "{B62ee-");

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
