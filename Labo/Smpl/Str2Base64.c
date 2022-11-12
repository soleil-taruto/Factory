#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	if (argIs("/S2B"))
	{
		char *sText;
		autoBlock_t *text;
		autoBlock_t *b64;
		char *sB64;

		sText = strx(nextArg());
		text = ab_fromLine(sText);
		b64 = encodeBase64(text);
		sB64 = ab_toLine(b64);

		cout("%s\n", sB64);

		memFree(sText);
		releaseAutoBlock(text);
		releaseAutoBlock(b64);
		memFree(sB64);
		return;
	}
	if (argIs("/B2S"))
	{
		char *sB64;
		autoBlock_t *b64;
		autoBlock_t *text;
		char *sText;

		sB64 = strx(nextArg());
		b64 = ab_fromLine(sB64);
		text = decodeBase64(b64);
		sText = ab_toLine(text);

		line2JLine(sText, 1, 1, 1, 1);

		cout("%s\n", sText);

		memFree(sB64);
		releaseAutoBlock(b64);
		releaseAutoBlock(text);
		memFree(sText);
		return;
	}
}
