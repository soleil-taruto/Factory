#include "C:\Factory\Common\all.h"
#include "libs\HTTPParse.h"

static uint GHV_Index;

static char *GetHeaderValue(char *targetKey)
{
	char *key;

	foreach (HttpDat.H_Keys, key, GHV_Index)
		if (!_stricmp(key, targetKey))
			return getLine(HttpDat.H_Values, GHV_Index);

	return NULL;
}
int main(int argc, char **argv)
{
	char *targetContentTypePtn;
	char *wContentType;
	char *wBodyFile;
	char *contentType;

	targetContentTypePtn = nextArg();
	wContentType = nextArg();
	wBodyFile = nextArg();

	cout("targetContentTypePtn: [%s]\n", targetContentTypePtn);
	cout("wContentType: [%s]\n", wContentType);
	cout("wBodyFile: [%s]\n", wBodyFile);

	LoadHttpDat(DEF_HTTP_DAT_FILE);

	contentType = GetHeaderValue("content-type");

	if (contentType && mbs_stristr(contentType, targetContentTypePtn))
	{
		LOGPOS();

		setElement(HttpDat.H_Values, GHV_Index, (uint)wContentType);
		HttpDat.Body = readBinary(wBodyFile);

		if (GetHeaderValue("content-length"))
			setElement(HttpDat.H_Values, GHV_Index, (uint)xcout("%u", getSize(HttpDat.Body)));

		SaveHttpDat(DEF_HTTP_DAT_FILE);
	}
	LOGPOS();
}
