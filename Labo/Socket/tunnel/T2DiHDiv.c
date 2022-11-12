/*
	T2DiHTTP.exe の出力ファイルを、ヘッダとコンテンツに分ける。

	- - -

	T2DiHDiv.exe [入力ディレクトリ]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Labo\Socket\libs\http\ExtToContentType.h"

static char *ContentType;

static void CheckHeaderLine(char *line)
{
	autoList_t *tokens = tokenizeMinMax(line, ':', 2, 2, "");
	char *name;
	char *value;

	name  = getLine(tokens, 0);
	value = getLine(tokens, 1);

	ucTrim(name);
	ucTrim(value);

	if (!_stricmp(name, "Content-Type"))
		strzp(&ContentType, value);

	releaseDim(tokens, 1);
}
static void ParseHTTPHdrBody(char *rFile)
{
	FILE *rfp = fileOpen(rFile, "rb");
	FILE *wfp;
	char *name;
	char *wHdrFile;
	char *wBdyFile;

	name = getLocal(rFile);
	name = changeExt(name, "");

	wHdrFile = getOutFile_x(xcout("%s_1-Head.txt", name));
	wfp = fileOpen(wHdrFile, "wb");

	strzp(&ContentType, "");

	for (; ; )
	{
		char *line = readLine(rfp);

		errorCase_m(!line, "ヘッダが終わる前にファイルの終端に達しました。");

		if (!*line)
		{
			memFree(line);
			break;
		}
		CheckHeaderLine(line);

		writeToken(wfp, line);
		writeToken(wfp, "\r\n");

		memFree(line);
	}
	fileClose(wfp);

	wBdyFile = getOutFile_x(xcout("%s_2-Body.%s", name, httpContentTypeToExt(ContentType)));
	wfp = fileOpen(wBdyFile, "wb");

	readWriteBinaryToEnd(rfp, wfp);

	fileClose(rfp);
	fileClose(wfp);

	memFree(name);
	memFree(wHdrFile);
	memFree(wBdyFile);
}
static void ProcFile(char *rFile)
{
	ParseHTTPHdrBody(rFile);
}
static void T2DiHDiv(char *dir)
{
	autoList_t *files = lsFiles(dir);
	char *file;
	uint index;

	foreach (files, file, index)
	{
		ProcFile(file);
	}
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	T2DiHDiv(hasArgs(1) ? nextArg() : c_dropDir());
	openOutDir();
}
