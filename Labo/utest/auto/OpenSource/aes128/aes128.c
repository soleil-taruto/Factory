#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\aes128.h"

static void EncDecTest(autoBlock_t *rawkey, autoBlock_t *plain, autoBlock_t *cipher)
{
	AES128_KeyTable_t *enckt;
	AES128_KeyTable_t *deckt;
	uchar encans[16];
	uchar decans[16];
	autoBlock_t gab;

	cout("Test\n");
	cout("K: %s\n", c_makeHexLine(rawkey));
	cout("P: %s\n", c_makeHexLine(plain));
	cout("C: %s\n", c_makeHexLine(cipher));

	errorCase(
		getSize(rawkey) != 16 //&&
//		getSize(rawkey) != 24 &&
//		getSize(rawkey) != 32
		);
	errorCase(getSize(plain) != 16);
	errorCase(getSize(cipher) != 16);

	enckt = AES128_CreateKeyTable(rawkey, 1);
	deckt = AES128_CreateKeyTable(rawkey, 0);
	AES128_EncryptBlock(enckt, plain,  gndBlockVar(encans, 16, gab));
	AES128_EncryptBlock(deckt, cipher, gndBlockVar(decans, 16, gab));

	cout("E: %s\n", c_makeHexLine(gndBlockVar(encans, 16, gab)));
	cout("D: %s\n", c_makeHexLine(gndBlockVar(decans, 16, gab)));

	errorCase(memcmp(encans, directGetBuffer(cipher), 16));
	errorCase(memcmp(decans, directGetBuffer(plain), 16));

	cout("Test OK\n");
}
static void Test_TestVector(void)
{
	FILE *fp = fileOpen("testvector\\t_aes128.txt", "rt");
	char *line;
	autoBlock_t *rawkey = NULL;

	while (line = readLine(fp))
	{
		if (startsWith(line, "K No."))
		{
			char *p = strchr(line, ':');

			if (rawkey)
				releaseAutoBlock(rawkey);

			errorCase(!p);
			p++;
			removeBlank(p);
			rawkey = makeBlockHexLine(p);

			cout("Key Changed\n");
		}
		else if (startsWith(line, "P No."))
		{
			autoBlock_t *plain;
			autoBlock_t *cipher;
			char *p;

			errorCase(!rawkey);

			p = strchr(line, ':');
			errorCase(!p);
			p++;
			removeBlank(p);
			plain = makeBlockHexLine(p);

			memFree(line);
			line = readLine(fp);
			errorCase(!line);
			errorCase(!startsWith(line, "C No."));

			p = strchr(line, ':');
			errorCase(!p);
			p++;
			removeBlank(p);
			cipher = makeBlockHexLine(p);

			EncDecTest(rawkey, plain, cipher);

			releaseAutoBlock(plain);
			releaseAutoBlock(cipher);
		}
		memFree(line);
	}
	errorCase(!rawkey); // ÉLÅ[Ç™àÍÇ¬Ç‡ñ≥Ç¢Ç¡ÇƒÇ±ÇΩÇ»Ç¢ÇæÇÎÅB
	releaseAutoBlock(rawkey);
}
int main(int argc, char **argv)
{
	Test_TestVector();
}
