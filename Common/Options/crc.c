#include "crc.h"

static uint CRC_FileFltr(char *file, uint (*startFunc)(void), uint (*updateFunc)(uint, void *, uint), uint (*finishFunc)(uint))
{
	FILE *fp = fileOpen(file, "rb");
	autoBlock_t *block;
	uint counter = startFunc();
	uint buffSize = 1024;

	while (block = readBinaryStream(fp, buffSize))
	{
		counter = updateFunc(counter, directGetBuffer(block), getSize(block));
		releaseAutoBlock(block);

		if (buffSize < 1024 * 1024 * 128)
			buffSize <<= 1;
	}
	fileClose(fp);
	return finishFunc(counter);
}

///////////
// crc-8 //
///////////

#define Rot8(counter) \
	do { \
	counter <<= 1; \
	if (counter & 0x100) { \
		counter ^= 0x107; \
	} \
	} while (0)

static uint Swap8(uint counter)
{
	Rot8(counter); // 1
	Rot8(counter); // 2
	Rot8(counter); // 3
	Rot8(counter); // 4
	Rot8(counter); // 5
	Rot8(counter); // 6
	Rot8(counter); // 7
	Rot8(counter); // 8

	return counter;
}
static uchar *RefSwap8Table(void)
{
	static uchar *table;
	uint counter;

	if (!table)
	{
		table = (uchar *)memAlloc(0x100);

		for (counter = 0x00; counter <= 0xff; counter++)
		{
			table[counter] = Swap8(counter);
		}
	}
	return table;
}

uint crc8Start(void)
{
	return 0x00;
}
uint crc8Update(uint counter, uint byte)
{
	errorCase(0x100 <= counter);
	errorCase(0x100 <= byte);

	return RefSwap8Table()[counter ^ byte];
}
uint crc8UpdateBlock(uint counter, void *block, uint blockSize)
{
	uchar *table = RefSwap8Table();
	uchar *p;

	errorCase(0x100 <= counter);

	for (p = (uchar *)block; p < (uchar *)block + blockSize; p++)
	{
		counter = table[counter ^ *p];
	}
	return counter;
}
uint crc8UpdateLine(uint counter, char *line)
{
	return crc8UpdateBlock(counter, line, strlen(line));
}
uint crc8Finish(uint counter)
{
	return counter;
}

uint crc8CheckBlock(void *block, uint blockSize)
{
	return crc8Finish(crc8UpdateBlock(crc8Start(), block, blockSize));
}
uint crc8CheckLine(char *line)
{
	return crc8Finish(crc8UpdateLine(crc8Start(), line));
}
uint crc8CheckFile(char *file)
{
	return CRC_FileFltr(file, crc8Start, crc8UpdateBlock, crc8Finish);
}

////////////
// crc-16 //
////////////

// crc-16 modbus ???

#define CRC16_POLY 0xa001

static uint16 *RefSwap16Table(void)
{
	static uint16 *table;
	uint i;

	if (!table)
	{
		table = (uint16 *)memAlloc(256 * sizeof(uint16));

		for (i = 0; i < 256; i++)
		{
			uint crc = i;
			uint c;

			for (c = 0; c < 8; c++)
			{
				if (crc & 1)
				{
					crc ^= CRC16_POLY << 1 | 1;
				}
				crc >>= 1;
			}
			table[i] = crc;
		}
	}
	return table;
}

uint crc16Start(void)
{
	return 0xffff;
}
uint crc16Update(uint counter, uint byte)
{
	return RefSwap16Table()[(counter ^ byte) & 0xff] ^ counter >> 8;
}
uint crc16UpdateBlock(uint counter, void *block, uint blockSize)
{
	uint16 *table = RefSwap16Table();
	uint index;

	for (index = 0; index < blockSize; index++)
	{
		counter = table[(counter ^ ((uchar *)block)[index]) & 0xff] ^ counter >> 8;
//		counter = crc16Update(counter, ((uchar *)block)[index]); // old_same
	}
	return counter;
}
uint crc16UpdateLine(uint counter, char *line)
{
	return crc16UpdateBlock(counter, line, strlen(line));
}
uint crc16Finish(uint counter)
{
	return counter;
//	return counter ^ 0xffff;
}
uint crc16CheckBlock(void *block, uint blockSize)
{
	return crc16Finish(crc16UpdateBlock(crc16Start(), block, blockSize));
}
uint crc16CheckLine(char *line)
{
	return crc16Finish(crc16UpdateLine(crc16Start(), line));
}
uint crc16CheckFile(char *file)
{
	return CRC_FileFltr(file, crc16Start, crc16UpdateBlock, crc16Finish);
}

////////////
// crc-32 //
////////////

static uint *CRC32Table;

static uint *MakeCRC32Table(void)
{
	uint *table;
	uint index;

	table = (uint *)memAlloc(256 * sizeof(uint));

	for (index = 0; index < 256; index++)
	{
		uint c = index;
		uint j;

		for (j = 0; j < 8; j++)
		{
			c = c & 1 ? 0xEDB88320 ^ c >> 1 : c >> 1;
		}
		table[index] = c;
	}
	return table;
}

uint crc32Start(void)
{
	return 0xFFFFFFFF;
}
uint crc32Update(uint counter, uint byte)
{
	uchar block[1];

	block[0] = byte;
	return crc32UpdateBlock(counter, block, 1);
}
uint crc32UpdateBlock(uint counter, void *block, uint blockSize)
{
	uint index;

	if (!CRC32Table)
		CRC32Table = MakeCRC32Table();

	for (index = 0; index < blockSize; index++)
	{
		counter = CRC32Table[(counter ^ ((uchar *)block)[index]) & 0xFF] ^ counter >> 8;
	}
	return counter;
}
uint crc32UpdateLine(uint counter, char *line)
{
	return crc32UpdateBlock(counter, line, strlen(line));
}
uint crc32Finish(uint counter)
{
	return counter ^ 0xFFFFFFFF;
}

uint crc32CheckBlock(void *block, uint blockSize)
{
	return crc32Finish(crc32UpdateBlock(crc32Start(), block, blockSize));
}
uint crc32CheckLine(char *line)
{
	return crc32Finish(crc32UpdateLine(crc32Start(), line));
}
uint crc32CheckFile(char *file)
{
	return CRC_FileFltr(file, crc32Start, crc32UpdateBlock, crc32Finish);
}

// ----

static void CIL_Print(char *jbuffer)
{
	cout("\r%79s\r%02x>%s", "", crc8CheckLine(jbuffer), jbuffer);
}
char *coInputLineCRC(void)
{
	return coInputLinePrn(CIL_Print);
}
