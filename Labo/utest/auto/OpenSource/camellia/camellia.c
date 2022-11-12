#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\camellia.h"

static void EncDecTest(autoBlock_t *rawkey, autoBlock_t *plain, autoBlock_t *cipher)
{
	camellia_KeyTableType kt;
	uchar encans[16];
	uchar decans[16];
	autoBlock_t gab;

	cout("Test\n");
	cout("K: %s\n", c_makeHexLine(rawkey));
	cout("P: %s\n", c_makeHexLine(plain));
	cout("C: %s\n", c_makeHexLine(cipher));

	errorCase(
		getSize(rawkey) != 16 &&
		getSize(rawkey) != 24 &&
		getSize(rawkey) != 32
		);
	errorCase(getSize(plain) != 16);
	errorCase(getSize(cipher) != 16);

	camellia_Ekeygen(getSize(rawkey) * 8, directGetBuffer(rawkey), kt);
	camellia_EncryptBlock(getSize(rawkey) * 8, directGetBuffer(plain), kt, encans);
	camellia_DecryptBlock(getSize(rawkey) * 8, directGetBuffer(cipher), kt, decans);

	cout("E: %s\n", c_makeHexLine(gndBlockVar(encans, 16, gab)));
	cout("D: %s\n", c_makeHexLine(gndBlockVar(decans, 16, gab)));

	errorCase(memcmp(encans, directGetBuffer(cipher), 16));
	errorCase(memcmp(decans, directGetBuffer(plain), 16));

	cout("Test OK\n");
}
static void Test_TestVector(void)
{
	FILE *fp = fileOpen("testvector\\t_camellia.txt", "rt");
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
