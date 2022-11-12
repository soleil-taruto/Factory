#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	if (!strcmp("1", getEnvLine("@NoPause")))
	{
		LOGPOS();
		return;
	}
	cout("‘±s‚·‚é‚É‚Í‰½‚©ƒL[‚ğ‰Ÿ‚µ‚Ä‰º‚³‚¢ . . .\n");
	clearGetKey();
}
