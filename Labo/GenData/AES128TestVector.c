#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\aes128.h"

static FILE *WFp;
static AES128_KeyTable_t *KeyTable;

static void WrTV_Line(int kind, uint no, autoBlock_t *block)
{
	uint index;

	errorCase(getSize(block) != 16); // 2bs

	writeToken_x(WFp, xcout("%c No.%03u : ", kind, no));

	for (index = 0; index < 16; index++)
		writeToken_x(WFp, xcout("%02X ", getByte(block, index)));

	writeChar(WFp, '\n');
}
static void WrTV(uint no, autoBlock_t *plain)
{
	autoBlock_t *cipher = bCreateBlock(16);

	AES128_EncryptBlock(KeyTable, plain, cipher);

	WrTV_Line('P', no, plain);
	WrTV_Line('C', no, cipher);
	writeLine(WFp, "");

	releaseAutoBlock(cipher);
}
static void WrTV_K(uint no, char *sRawKey)
{
	autoBlock_t *rawKey = makeBlockHexLine(sRawKey);
	uint bit;

	WrTV_Line('K', no, rawKey);
	writeLine(WFp, "");

	KeyTable = AES128_CreateKeyTable(rawKey, 1);

	for (bit = 0; bit < 128; bit++)
	{
		autoBlock_t *plain = bCreateBlock(16);

		setByte(plain, bit / 8, 0x80 >> bit % 8);

		WrTV(bit + 1, plain);

		releaseAutoBlock(plain);
	}
	AES128_ReleaseKeyTable(KeyTable);
	releaseAutoBlock(rawKey);
}
static void Main2(void)
{
	WFp = fileOpen(c_getOutFile("t_aes128.txt"), "wt");

	writeLine(WFp, "AES with 128-bit key");
	writeLine(WFp, "");

	WrTV_K( 1, "00000000000000000000000000000000");
	WrTV_K( 2, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
	WrTV_K( 3, "0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F");
	WrTV_K( 4, "F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0F0");
	WrTV_K( 5, "00112233445566778899AABBCCDDEEFF");
	WrTV_K( 6, "FFEEDDCCBBAA99887766554433221100");
	WrTV_K( 7, "0123456789ABCDEFFEDCBA9876543210");
	WrTV_K( 8, "FEDCBA98765432100123456789ABCDEF");
	WrTV_K( 9, "1032547698BADCFEEFCDAB8967452301");
	WrTV_K(10, "EFCDAB89674523011032547698BADCFE");

	fileClose(WFp);
	openOutDir();
}
int main(int argc, char **argv)
{
	Main2();
}
