/*
	CamelliaRingCBC.exe /K 鍵ファイル /R 入力ファイル (/EB | /DB | /E | /D) /W 出力ファイル
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\RingCipher2.h"

int main(int argc, char **argv)
{
	autoBlock_t *rawKey = NULL;
	autoBlock_t *rData = NULL;
	autoBlock_t *wData;
	int mode = 0; // "EDed"
	char *wFile = NULL;
	autoList_t *keyTableList;

readArgs:
	if (argIs("/K"))
	{
		errorCase(rawKey);

		rawKey = readBinary(nextArg());
		goto readArgs;
	}
	if (argIs("/R"))
	{
		errorCase(rData);

		rData = readBinary(nextArg());
		goto readArgs;
	}
	if (argIs("/EB"))
	{
		mode = 'E';
		goto readArgs;
	}
	if (argIs("/DB"))
	{
		mode = 'D';
		goto readArgs;
	}
	if (argIs("/E"))
	{
		mode = 'e';
		goto readArgs;
	}
	if (argIs("/D"))
	{
		mode = 'd';
		goto readArgs;
	}
	if (argIs("/W"))
	{
		errorCase(wFile);

		wFile = nextArg();
		goto readArgs;
	}

	errorCase(!rawKey);
	errorCase(!rData);
	errorCase(!mode);
	errorCase(!wFile);

	keyTableList = rngcphrCreateKeyTableList(rawKey);
	wData = copyAutoBlock(rData);
	(	mode == 'E' ? rngcphrEncryptBlock :
		mode == 'D' ? rngcphrDecryptBlock :
		mode == 'e' ? rngcphrEncrypt : rngcphrDecrypt )(wData, keyTableList);

	writeBinary(wFile, wData);

	releaseAutoBlock(rawKey);
	releaseAutoBlock(rData);
	releaseAutoBlock(wData);
//	mode
//	wFile
	cphrReleaseKeyTableList(keyTableList);
}
