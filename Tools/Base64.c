/*
	Base64.exe /ToBase64   ���̓t�@�C�� �o�̓t�@�C��
	Base64.exe /FromBase64 ���̓t�@�C�� �o�̓t�@�C��
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	if (argIs("/ToBase64"))
	{
		autoBlock_t *text;
		autoBlock_t *b64;

		text = readBinary(nextArg());
		b64 = encodeBase64(text);
		writeBinary(nextArg(), b64);

		releaseAutoBlock(text);
		releaseAutoBlock(b64);
		return;
	}
	if (argIs("/FromBase64"))
	{
		autoBlock_t *b64;
		autoBlock_t *text;

		b64 = readBinary(nextArg());
		text = decodeBase64(b64);
		writeBinary(nextArg(), text);

		releaseAutoBlock(b64);
		releaseAutoBlock(text);
		return;
	}
}
