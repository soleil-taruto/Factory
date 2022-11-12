#include "bmp.h"

#define PIXNUMMAX 0x50000000 // UINTMAX / 3 あたり、GetSizeImage() を考慮しただけ。

static struct
{
	uint16 Type;
	uint Size;
	uint16 Reserved_01;
	uint16 Reserved_02;
	uint OffBits;
}
Bfh;

static struct
{
	uint Size;
	uint Width;
	uint Height;
	uint16 Planes;
	uint16 BitCount;
	uint Compression;
	uint SizeImage;
	uint XPelsPerMeter;
	uint YPelsPerMeter;
	uint ClrUsed;
	uint ClrImportant;
}
Bfi;

/*
	フォーマットエラー -> error();

	ret: 解放するには、releaseDim_BR(bmp, 2, NULL);
*/
autoList_t *readBMPFile(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	autoList_t *table = newList();
	autoList_t *colorPallet = newList();
	uint colPalCnt;
	uint x;
	uint y;
	uint hiSign;

	Bfh.Type = readValueWidth(fp, 2); // 'BM'
	Bfh.Size = readValue(fp);
	Bfh.Reserved_01 = readValueWidth(fp, 2);
	Bfh.Reserved_02 = readValueWidth(fp, 2);
	Bfh.OffBits = readValue(fp);

	Bfi.Size = readValue(fp);
	Bfi.Width = readValue(fp);
	Bfi.Height = readValue(fp);
	Bfi.Planes = readValueWidth(fp, 2);
	Bfi.BitCount = readValueWidth(fp, 2);
	Bfi.Compression = readValue(fp);
	Bfi.SizeImage = readValue(fp);
	Bfi.XPelsPerMeter = readValue(fp);
	Bfi.YPelsPerMeter = readValue(fp);
	Bfi.ClrUsed = readValue(fp);
	Bfi.ClrImportant = readValue(fp);

	errorCase(Bfh.Type != ('B' | 'M' << 8));

	hiSign = 0x80000000 & Bfi.Height;

	if (hiSign)
		Bfi.Height = (Bfi.Height ^ 0xffffffff) + 1;

	errorCase(!Bfi.Width);
	errorCase(!Bfi.Height);
	errorCase(PIXNUMMAX / Bfi.Width < Bfi.Height); // HACK: これ必要か？

	switch (Bfi.BitCount)
	{
	case 1: colPalCnt = 2; break;
	case 4: colPalCnt = 16; break;
	case 8: colPalCnt = 256; break;

	case 24:
	case 32:
		colPalCnt = 0;
		break;

	default:
		error();
	}
	while (colPalCnt)
	{
		uchar cR;
		uchar cG;
		uchar cB;

		// BGR 注意
		cB = readChar(fp);
		cG = readChar(fp);
		cR = readChar(fp);
		readChar(fp); // reserved

		addElement(colorPallet, cR << 16 | cG << 8 | cB);

		colPalCnt--;
	}
	for (y = 0; y < Bfi.Height; y++)
	{
		autoList_t *row = newList();

		if (Bfi.BitCount <= 8)
		{
			uint bitMax = 8 / Bfi.BitCount;
			uint bit;

			x = 0;

			while (x < Bfi.Width)
			{
				uchar c8 = readChar(fp);

				for (bit = bitMax; bit && x < Bfi.Width; bit--, x++)
				{
					addElement(row, getElement(colorPallet, c8 & (1 << Bfi.BitCount) - 1));
					c8 >>= Bfi.BitCount;
				}
			}
			for (x = (4 - ((Bfi.Width + bitMax - 1) / bitMax) % 4) % 4; x; x--)
			{
				readChar(fp);
			}
		}
		else if (Bfi.BitCount == 24)
		{
			for (x = 0; x < Bfi.Width; x++)
			{
				uchar cR;
				uchar cG;
				uchar cB;

				// BGR 注意
				cB = readChar(fp);
				cG = readChar(fp);
				cR = readChar(fp);

				addElement(row, cR << 16 | cG << 8 | cB);
			}
			for (x = Bfi.Width % 4; x; x--)
			{
				readChar(fp);
			}
		}
		else // ? Bfi.BitCount == 32
		{
			for (x = 0; x < Bfi.Width; x++)
			{
				uchar cR;
				uchar cG;
				uchar cB;

				// BGR 注意
				cB = readChar(fp);
				cG = readChar(fp);
				cR = readChar(fp);
				readChar(fp); // reserved

				addElement(row, cR << 16 | cG << 8 | cB);
			}
		}
		addElement(table, (uint)row);
	}
	fileClose(fp);

	if (!hiSign)
		reverseElements(table); // 0 <= Bfi.Height なら、下から上 (左から右) に並ぶ。

	releaseAutoList(colorPallet);
	return table;
}
static uint GetSizeImage(uint xSize, uint ySize)
{
	return ((xSize * 3 + 3) / 4) * 4 * ySize;
}
void writeBMPFile(char *file, autoList_t *table)
{
	FILE *fp = fileOpen(file, "wb");
	uint xSize;
	uint ySize;
	uint x;
	uint y;

	xSize = getCount((autoList_t *)getElement(table, 0));
	ySize = getCount(table);

	errorCase(!xSize);
//	errorCase(!ySize);
	errorCase(PIXNUMMAX / xSize < ySize);

	// Bfh
	writeChar(fp, 'B');
	writeChar(fp, 'M');
	writeValue(fp, GetSizeImage(xSize, ySize) + 0x36);
	writeValue(fp, 0); // Reserved_01 + Reserved_02
	writeValue(fp, 0x36);

	// Bfi
	writeValue(fp, 0x28);
	writeValue(fp, xSize);
	writeValue(fp, ySize);
	writeValue(fp, 0x00180001); // Planes + BitCount
	writeValue(fp, 0);
	writeValue(fp, GetSizeImage(xSize, ySize));
	writeValue(fp, 0);
	writeValue(fp, 0);
	writeValue(fp, 0);
	writeValue(fp, 0);

	for (y = ySize; y; y--)
	{
		autoList_t *row = (autoList_t *)getElement(table, y - 1);

		for (x = 0; x < xSize; x++)
		{
			uint color = getElement(row, x);
			uchar cR;
			uchar cG;
			uchar cB;

			cR = color >> 16;
			cG = color >> 8;
			cB = color;

			// BGR 注意
			writeChar(fp, cB);
			writeChar(fp, cG);
			writeChar(fp, cR);
		}
		for (x = xSize % 4; x; x--)
		{
			writeChar(fp, 0x00);
		}
	}
	fileClose(fp);
}

// _x
void writeBMPFile_cx(char *file, autoList_t *table)
{
	writeBMPFile(file, table);
	releaseDim_BR(table, 2, NULL);
}
void writeBMPFile_xx(char *file, autoList_t *table)
{
	writeBMPFile_cx(file, table);
	memFree(file);
}
