#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"

static void DecodeJP(char *file)
{
	char *wkFile = makeTempPath(NULL);

	UTF8ToSJISFile(file, file);

	UTF_BE = 0;
	UTF_NoWriteBOM = 1;
	SJISToUTF16File(file, wkFile);

	{
		autoBlock_t *text = readBinary(wkFile);
		uint index;
		autoBlock_t *dest = newBlock();

		for (index = 0; index + 2 <= getSize(text); index += 2)
		{
			if (
				index + 4 <= getSize(text) &&
				b_(text)[index + 0] == '\\' && b_(text)[index + 1] == 0x00 &&
				b_(text)[index + 2] == '\\' && b_(text)[index + 3] == 0x00
				)
			{
				addByte(dest, '\\');
				addByte(dest, 0x00);
				addByte(dest, '\\');
				addByte(dest, 0x00);

				index += 2;
			}
			else if (
				index + 12 <= getSize(text) &&
				b_(text)[index + 0] == '\\' && b_(text)[index + 1] == 0x00 &&
				b_(text)[index + 2] == 'u'  && b_(text)[index + 3] == 0x00 &&
				m_ishexadecimal(b_(text)[index +  4]) && b_(text)[index +  5] == 0x00 &&
				m_ishexadecimal(b_(text)[index +  6]) && b_(text)[index +  7] == 0x00 &&
				m_ishexadecimal(b_(text)[index +  8]) && b_(text)[index +  9] == 0x00 &&
				m_ishexadecimal(b_(text)[index + 10]) && b_(text)[index + 11] == 0x00
				)
			{
				uint u16ch =
					m_c2i(b_(text)[index +  4]) << 12 |
					m_c2i(b_(text)[index +  6]) <<  8 |
					m_c2i(b_(text)[index +  8]) <<  4 |
					m_c2i(b_(text)[index + 10]);

				addByte(dest, u16ch & 0xff);
				addByte(dest, u16ch >> 8);

				index += 10;
			}
			else
			{
				addByte(dest, b_(text)[index + 0]);
				addByte(dest, b_(text)[index + 1]);
			}
		}
		writeBinary(wkFile, dest);

		releaseAutoBlock(text);
		releaseAutoBlock(dest);
	}

	UTF_BE = 0;
	UTF16ToSJISFile(wkFile, file);

	removeFile_x(wkFile);
}
static void EncodeJP(char *file)
{
	char *wkFile = makeTempPath(NULL);

	UTF_BE = 0;
	UTF_NoWriteBOM = 1;
	SJISToUTF16File(file, wkFile);

	{
		autoBlock_t *text = readBinary(wkFile);
		uint index;
		autoBlock_t *dest = newBlock();

		for (index = 0; index + 2 <= getSize(text); index += 2)
		{
			uint u16ch = b_(text)[index] | b_(text)[index + 1] << 8;

			if (
				u16ch == '\t' ||
				u16ch == '\r' ||
				u16ch == '\n' ||
				m_isascii(u16ch)
				)
			{
				addByte(dest, u16ch & 0xff);
				addByte(dest, u16ch >> 8);
			}
			else
			{
				addByte(dest, '\\');
				addByte(dest, 0x00);
				addByte(dest, 'u');
				addByte(dest, 0x00);
				addByte(dest, hexadecimal[u16ch >> 12 & 0x0f]);
				addByte(dest, 0x00);
				addByte(dest, hexadecimal[u16ch >> 8 & 0x0f]);
				addByte(dest, 0x00);
				addByte(dest, hexadecimal[u16ch >> 4 & 0x0f]);
				addByte(dest, 0x00);
				addByte(dest, hexadecimal[u16ch & 0x0f]);
				addByte(dest, 0x00);
			}
		}
		writeBinary(wkFile, dest);

		releaseAutoBlock(text);
		releaseAutoBlock(dest);
	}

	UTF_BE = 0;
	UTF16ToSJISFile(wkFile, file);

	SJISToUTF8File(file, file);

	removeFile_x(wkFile);
}
static void Main2(void)
{
	char *file = dropFile();
	char *wkDir = makeTempDir(NULL);
	char *wkFile;
	char *bkFile;

	wkFile = combine(wkDir, getLocal(file));
	bkFile = makeTempPath(NULL);

	copyFile(file, wkFile);
	DecodeJP(wkFile);
	copyFile(wkFile, bkFile);

	editTextFile(wkFile);

	if (!isSameFile(wkFile, bkFile))
	{
		cout("ï“èWÇ≥ÇÍÇΩÇÃÇ≈ï€ë∂ÇµÇ‹Ç∑ÅB\n");

		EncodeJP(wkFile);
		copyFile(wkFile, file);
	}
	removeFile_x(wkFile);
	removeFile_x(bkFile);
	removeDir_x(wkDir);
	memFree(file);
}
int main(int argc, char **argv)
{
	Main2();
}
