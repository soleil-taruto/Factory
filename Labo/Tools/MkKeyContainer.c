#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand_MS.h"

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/C"))
	{
		cout("CREATE\n");
		createKeyContainer();
		goto readArgs;
	}
	if (argIs("/D"))
	{
		cout("DELETE\n");
		deleteKeyContainer();
		goto readArgs;
	}
}
