#include "C:\Factory\Common\all.h"

static int IsUSAscii(int chr)
{
	return m_isRange(chr, 0x21, 0x7e);
}
static int IsHanKana(int chr)
{
	return m_isRange(chr, 0xa1, 0xdf);
}
static int My_IsJChar(int chr)
{
	return IsUSAscii(chr) || IsHanKana(chr) || isJChar(chr);
}
int main(int argc, char **argv)
{
	FILE *fp = fileOpen(c_getOutFile("JChar.bin"), "wb");
	int chr;

	for (chr = 0; chr <= 0xffff; chr += 8)
	{
		int bits = 0;
		int bit;

		for (bit = 0; bit < 8; bit++)
		{
			if (My_IsJChar(chr + bit))
			{
				bits |= 1 << bit;
			}
		}
		writeChar(fp, bits);
	}
	fileClose(fp);

	openOutDir();
}
