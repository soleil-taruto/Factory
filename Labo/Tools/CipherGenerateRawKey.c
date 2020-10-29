/*
	Cipher.exe 入力ファイル 出力ファイル

		入力ファイル ... テキスト(SJIS)
		出力ファイル ... バイナリ

	----

	CamelliaRingCipherUtils.generateRawKey() と同じ動きをする。

	CamelliaRingCipherUtilsTest.test01() で使用する。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Cipher.h"

int main(int argc, char **argv)
{
	char *rFile;
	char *wFile;
	char *passphrase;
	autoBlock_t *keyBundle;

	rFile = nextArg();
	wFile = nextArg();

	passphrase = readText_b(rFile);
//	cout("< %s\n", passphrase);
	passphrase = insertChar(passphrase, 0, '*');

	keyBundle = cphrLoadKeyBundleFileEx(passphrase);

	/*
	{
		char *tmp = ab_toHexLine(keyBundle);
		cout("> %s\n", tmp);
		memFree(tmp);
	}
	*/

	writeBinary(wFile, keyBundle);

	memFree(passphrase);
	releaseAutoBlock(keyBundle);
}
