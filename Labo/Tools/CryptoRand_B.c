/*
	Used by C:\Factory\Labo\utest\auto\Common\Options\CryptoRand.c
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

int main(int argc, char **argv)
{
	if (hasArgs(2))
		writeBinary_cx(getArg(1), makeCryptoRandBlock(toValue(getArg(0))));
	else
		cout("%s\n", ab_toHexLine_x(makeCryptoRandBlock(toValue(nextArg())))); // g
}
