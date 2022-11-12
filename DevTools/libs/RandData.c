#include "RandData.h"

#define FILEBLOCKSIZE (128 * 1024 * 1024)

// Def
int MRLChrLow   = 0x21;
int MRLChrUpper = 0x7e;

int MRLGetCharRange(void)
{
	return mt19937_range(MRLChrLow, MRLChrUpper);
}
int (*MRLGetChar)(void) = MRLGetCharRange;

char *MRLChrSet;

int MRLGetCharSet(void)
{
	return MRLChrSet[mt19937_rnd(strlen(MRLChrSet))];
}

char *MakeRandLine(uint count)
{
	char *line = (char *)memAlloc(count + 1);
	uint index;

	for (index = 0; index < count; index++)
	{
		line[index] = MRLGetChar();
	}
	line[index] = '\0';
	return line;
}
char *MakeRandLineRange(uint lenmin, uint lenmax)
{
	return MakeRandLine(mt19937_range(lenmin, lenmax));
}
autoList_t *MakeRandTextLinesRange(uint row, uint lenmin, uint lenmax)
{
	autoList_t *lines = createAutoList(row);
	char *line;
	uint index;

	for (index = 0; index < row; index++)
	{
		addElement(lines, (uint)MakeRandLineRange(lenmin, lenmax));
	}
	return lines;
}
autoList_t *MakeRandTextLines(uint row, uint column)
{
	return MakeRandTextLinesRange(row, column, column);
}
void MakeRandTextFileRange(char *file, uint64 rowCounter, uint lenmin, uint lenmax)
{
	FILE *fp = fileOpen(file, "wt");
	char *line;

	for (; 0ui64 < rowCounter; rowCounter--)
	{
		line = MakeRandLineRange(lenmin, lenmax);
		writeLine(fp, line);
		memFree(line);
	}
	fileClose(fp);
}
void MakeRandTextFile(char *file, uint64 rowCounter, uint column)
{
	MakeRandTextFileRange(file, rowCounter, column, column);
}

autoBlock_t *MakeRandBinaryBlock(uint size)
{
	uchar *block = (uchar *)memAlloc(size + sizeof(uint) - 1);
	uint index;

	for (index = 0; index < size; index += sizeof(uint))
	{
		*(uint *)(block + index) = mt19937_rnd32();
	}
	return bindBlock(block, size);
}
void MakeRandBinaryFile(char *file, uint64 fileSize)
{
	FILE *fp = fileOpen(file, "wb");

	while (0i64 < fileSize)
	{
		uint blockSize = fileSize < FILEBLOCKSIZE ? (uint)fileSize : FILEBLOCKSIZE;
		autoBlock_t *block;

		block = MakeRandBinaryBlock(blockSize);
		writeBinaryBlock(fp, block);
		releaseAutoBlock(block);

		fileSize -= blockSize;
	}
	fileClose(fp);
}

void RandXorOneBit(autoBlock_t *block)
{
	uint index = mt19937_rnd(getSize(block));
	uint bit = mt19937_rnd(8);

	setByte(block, index, getByte(block, index) ^ 0x01 << bit);
}
void RandXorOneBitFile_RW(char *rFile, char *wFile)
{
	uint64 fileSize = getFileSize(rFile);
	uint64 index;
	uint bit = mt19937_rnd(8);
	uint64 count;
	FILE *rfp;
	FILE *wfp;

	index = mt19937_rnd64Mod(fileSize);

	rfp = fileOpen(rFile, "rb");
	wfp = fileOpen(wFile, "wb");

	for (count = 0; count < fileSize; count++)
	{
		int chr = readChar(rfp);

		if (count == index)
		{
			chr ^= 0x01 << bit;
		}
		writeChar(wfp, chr);
	}
	fileClose(rfp);
	fileClose(wfp);
}
void RandXorOneBitFile(char *file)
{
	char *tmpFile = makeTempPath("rand-data");

	moveFile(file, tmpFile);
	RandXorOneBitFile_RW(tmpFile, file);
	removeFile(tmpFile);
	memFree(tmpFile);
}
