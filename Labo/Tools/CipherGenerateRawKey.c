/*
	Cipher.exe ���̓t�@�C�� �o�̓t�@�C��

		���̓t�@�C�� ... �e�L�X�g(SJIS)
		�o�̓t�@�C�� ... �o�C�i��

	----

	CamelliaRingCipherUtils.generateRawKey() �Ɠ�������������B

	CamelliaRingCipherUtilsTest.test01() �Ŏg�p����B
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
