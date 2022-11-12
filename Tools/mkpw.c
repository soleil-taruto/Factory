/*
	mkpw -> 62 P 22 L 2 = 130.*
	mkpu -> 36 P 25 L 2 = 129.*
	mkpd -> 10 P 39 L 2 = 129.*
	mkpd -> 10 P 40 L 2 = 132.*
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

static int CheckPw(char *pw, uint pwChrNum)
{
	int dig = 0;
	int upr = 1;
	int lwr = 1;
	char *p;

	switch (pwChrNum)
	{
	case 62: lwr = 0; // fall throught
	case 36: upr = 0; // fall throught
	case 10:
		break;

	default:
		error();
	}
	for (p = pw; *p; p++)
	{
		   if (m_isdecimal(*p)) dig = 1;
		else if (m_isupper(*p)) upr = 1;
		else if (m_islower(*p)) lwr = 1;
	}
	return dig && upr && lwr;
}
int main(int argc, char **argv)
{
	uint pwChrNum = 62;
	uint pwLen = 22;
	uint chrIdx;
	int noCheck = 0;
	int noView = 0;
	int forceLower = 0;
	char *pwChrs;
	char *pw;
	int chr;

	if (argIs("/9"))
	{
		pwChrNum = 10;
//		pwLen = 39;
		pwLen = 40; // åÖêîÇÃêÿÇËÇ™ó«Ç¢ÇÃÇ≈ÅAÇ±ÇøÇÁÇégÇ§ÅB
	}
	if (argIs("/9U"))
	{
		pwChrNum = 36;
		pwLen = 25;
	}
	if (argIs("/9L"))
	{
		pwChrNum = 36;
		pwLen = 25;
		forceLower = 1;
	}
	if (argIs("/9UL") || argIs("/9A"))
	{
		pwChrNum = 62;
		pwLen = 22;
	}
	if (argIs("/-C"))
	{
		noCheck = 1;
	}
	if (argIs("/-V"))
	{
		noView = 1;
	}
	if (hasArgs(1))
	{
		pwLen = toValue(nextArg());
		errorCase(pwLen < 3);
	}

	pwChrs = strx("");

	for (chr = '0'; chr <= '9'; chr++) pwChrs = addChar(pwChrs, chr);
	for (chr = 'A'; chr <= 'Z'; chr++) pwChrs = addChar(pwChrs, chr);
	for (chr = 'a'; chr <= 'z'; chr++) pwChrs = addChar(pwChrs, chr);

	errorCase(strlen(pwChrs) != 62); // 2bs

	pw = strx("");

	for (; ; )
	{
		while (strlen(pw) < pwLen)
		{
			do
			{
				chrIdx = getCryptoByte() & 0x3f;
			}
			while (pwChrNum <= chrIdx);

			pw = addChar(pw, pwChrs[chrIdx]);
		}
		if (noCheck)
			break;

		if (CheckPw(pw, pwChrNum))
			break;

		pw[0] = '\0';
	}
	if (forceLower)
		toLowerLine(pw);

	cout("%s\n", pw);

	if (!noView)
		viewLine(pw);

	memFree(pwChrs);
	memFree(pw);
}
