/*
	使用例

		HTTPProxy 80 a 1 /f- "HTTPPSimpleSaver.exe C:\temp"
		SimpleProxy /f- HTTPPSimpleSaver.exe
*/

#include "C:\Factory\Common\all.h"
#include "libs\HTTPParse.h"
#include "C:\Factory\Labo\Socket\libs\http\ExtToContentType.h"

#define MARGIN_PTN "000"
//#define MARGIN_PTN "0000"
//#define MARGIN_PTN "00000"

static char *GetContentType(void)
{
	char *line;
	uint index;

	// DEF_HTTP_DAT_FILE は再形成なので、きっちり KEY + ": " + VALUE になっているはず。

	foreach (HttpDat.Header, line, index)
		if (startsWithICase(line, "Content-Type: "))
			return strchr(line, ':') + 2;

	return ""; // not found
}
static char *GetOutExt(void)
{
	return httpContentTypeToExt(GetContentType());
}
int main(int argc, char **argv)
{
	char *outDir;
	char *outFile;

	if (hasArgs(1))
		outDir = nextArg();
	else
		outDir = "C:\\temp";

	LoadHttpDat(DEF_HTTP_DAT_FILE);

	outFile = combine(outDir, xcout("%s" MARGIN_PTN ".%s", makeCompactStamp(NULL), GetOutExt()));
	outFile = toCreatablePath(outFile, IMAX);

	writeBinary(outFile, HttpDat.Body);

	memFree(outFile);
}
