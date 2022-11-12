#include "C:\Factory\Common\all.h"

static void BITxtIncrement_File(char *file)
{
	autoBlock_t *data = newBlock();

	{
		FILE *fp = fileOpen(file, "rt");

		for (; ; )
		{
			int chr = readChar(fp);

			if (chr == EOF)
				break;

			if (m_isdecimal(chr))
				addByte(data, (uint)(chr - '0'));
		}
		fileClose(fp);
	}

	reverseBytes(data);

	{
		uint value = 1;
		uint index;

		for (index = 0; value; index++)
		{
			value += refByte(data, index);
			putByte(data, index, value % 10);
			value /= 10;
		}
	}

	while (getSize(data) && getByte(data, getSize(data) - 1) == 0)
		setSize(data, getSize(data) - 1);

	reverseBytes(data);

	{
		FILE *fp = fileOpen(file, "wt");
		uint index;

		for (index = 0; index < getSize(data); index++)
			writeChar(fp, (int)getByte(data, index) + '0');

		fileClose(fp);
	}
}
int main(int argc, char **argv)
{
	BITxtIncrement_File(nextArg());
}
