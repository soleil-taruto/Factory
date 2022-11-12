#include "Tools.h"

char *GetSHA512_128String(char *str)
{
	sha512_makeHashLine(str);
	sha512_makeHexHash();
	sha512_hexHash[32] = '\0';

	return strx(sha512_hexHash);
}
char *GetTmp(void)
{
	static char *dir;

	if (!dir)
		dir = strx(getEnvLine("TMP"));

	return dir;
}
