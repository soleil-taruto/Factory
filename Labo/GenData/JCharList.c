#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	FILE *fp = fileOpen(c_getOutFile("JCharList.c.txt"), "wt");
	uint index;
	uint bit;

	writeLine(fp, "uint JCharList[] =");
	writeLine(fp, "{");

	for (index = 0; index < 0x0800; index++)
	{
		char sBits[33];
		uint value = 0;

		for (bit = 0; bit < 0x0020; bit++)
		{
			int chr = index << 5 | bit;

			if (isJChar(chr))
			{
				sBits[bit] = '1';
				value |= 1u << bit;
			}
			else
			{
				sBits[bit] = '0';
			}
		}
		sBits[32] = '\0';

		writeLine(fp, xcout("\t0x%08x, // %04x: %s", value, index, sBits));
	}
	writeLine(fp, "};");

	fileClose(fp);

	openOutDir();
}
