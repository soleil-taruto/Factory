/*
	urlDecoder.exe [/E] ...

		/E ... エンコード

	urlDecoder.exe ... [R-FILE [W-FILE]]

		R-FILE ... 入力ファイル
		W-FILE ... 出力ファイル
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\URL.h"

static int EncodeMode;

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/E"))
	{
		EncodeMode = 1;
		goto readArgs;
	}

	if (hasArgs(2))
	{
		char *rFile;
		char *wFile;
		char *rText;
		char *wText;

		rFile = nextArg();
		wFile = nextArg();

		rText = unbindBlock2Line(readBinary(rFile));

		if (!*rText)
			termination(0);

		if (EncodeMode)
			wText = urlEncoder(rText);
		else
			wText = urlDecoder(rText);

		writeOneLineNoRet_b(wFile, wText);

		memFree(rText);
		memFree(wText);
		return;
	}
	if (hasArgs(1))
	{
		char *rFile = nextArg();
		char *wFile;
		char *rText;
		char *wText;

		rText = unbindBlock2Line(readBinary(rFile));

		if (!*rText)
			termination(0);

		wFile = getOutFile("urlDecOut.txt");

		if (EncodeMode)
			wText = urlEncoder(rText);
		else
			wText = urlDecoder(rText);

		writeOneLineNoRet_b(wFile, wText);
		coExecute_x(xcout("START \"\" \"%s\"", wFile));

		memFree(rText);
		memFree(wText);
		memFree(wFile);
		return;
	}

	{
		char *rText = unbindBlock2Line(inputTextAsBinary());
		char *wText;
		char *wFile;

		if (!*rText)
			termination(0);

		wFile = getOutFile("urlDecOut.txt");

		if (EncodeMode)
			wText = urlEncoder(rText);
		else
			wText = urlDecoder(rText);

		writeOneLineNoRet_b(wFile, wText);
		coExecute_x(xcout("START \"\" \"%s\"", wFile));

		memFree(rText);
		memFree(wText);
		memFree(wFile);
	}
}
