#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/BE"))
	{
		UTF_BE = 1;
		goto readArgs;
	}
	if (argIs("/NB"))
	{
		UTF_NoWriteBOM = 1;
		goto readArgs;
	}
	if (argIs("/JIS0208")) // ëΩï™à”ñ°ñ≥Ç¢ÅB
	{
		UTF_UseJIS0208 = 1;
		goto readArgs;
	}

	if (argIs("/16S"))
	{
		UTF16ToSJISFile(getArg(0), getArg(1));
		return;
	}
	if (argIs("/S16"))
	{
		SJISToUTF16File(getArg(0), getArg(1));
		return;
	}
	if (argIs("/816"))
	{
		UTF8ToUTF16File(getArg(0), getArg(1));
		return;
	}
	if (argIs("/168"))
	{
		UTF16ToUTF8File(getArg(0), getArg(1));
		return;
	}
	if (argIs("/8S"))
	{
		UTF8ToSJISFile(getArg(0), getArg(1)); // arg_0 == arg_1 ok
		return;
	}
	if (argIs("/S8"))
	{
		SJISToUTF8File(getArg(0), getArg(1)); // arg_0 == arg_1 ok
		return;
	}
	if (argIs("/816"))
	{
		UTF8ToUTF16File(getArg(0), getArg(1));
		return;
	}
	if (argIs("/168"))
	{
		UTF16ToUTF8File(getArg(0), getArg(1));
		return;
	}
}
