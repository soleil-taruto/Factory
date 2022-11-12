#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\sha512.h"

int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		sha512_makeHashBlock(c_makeBlockHexLine(nextArg()));
		sha512_makeHexHash();

		cout("%s\n", sha512_hexHash);
		return;
	}

	{
		sha512_makeHashBlock(c_makeBlockHexLine_x(inputLine()));
		sha512_makeHexHash();

		cout("%s\n", sha512_hexHash);
	}
}
