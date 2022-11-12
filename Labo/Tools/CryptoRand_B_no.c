#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

int main(int argc, char **argv)
{
#if 1
	uint c;

	for (c = toValue(nextArg()); c; c--)
	{
		getCryptoByte();
	}
#else
	makeCryptoRandBlock(toValue(nextArg())); // g
#endif
}
